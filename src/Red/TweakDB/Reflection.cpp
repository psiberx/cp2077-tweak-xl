#include "Reflection.hpp"
#include "Red/TweakDB/Source/Grammar.hpp"
#include "Red/TweakDB/Source/Source.hpp"

namespace
{
constexpr auto NameSeparator = Red::TweakGrammar::Name::Separator;
constexpr auto PropSeparator = std::string_view(NameSeparator);
} // namespace

namespace Red
{

TweakDBReflection::TweakDBReflection()
    : TweakDBReflection(TweakDB::Get())
{
}

TweakDBReflection::TweakDBReflection(TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
    , m_rtti(CRTTISystem::Get())
{
}

const TweakDBRecordInfo* TweakDBReflection::GetRecordInfo(const CClass* aType)
{
    if (!IsRecordType(aType))
        return nullptr;

    {
        std::shared_lock lockR(m_mutex);
        if (const auto iter = m_resolved.find(aType->GetName()); iter != m_resolved.end())
            return iter->second.get();
    }

    return CollectRecordInfo(aType).get();
}

const TweakDBRecordInfo* TweakDBReflection::GetRecordInfo(CName aTypeName)
{
    {
        std::shared_lock lockR(m_mutex);
        if (const auto iter = m_resolved.find(aTypeName); iter != m_resolved.end())
            return iter->second.get();
    }

    return CollectRecordInfo(m_rtti->GetClass(aTypeName)).get();
}

Core::SharedPtr<TweakDBRecordInfo> TweakDBReflection::CollectRecordInfo(const CClass* aType)
{
    if (!IsRecordType(aType))
        return nullptr;

    auto recordInfo = MakeInstance<TweakDBRecordInfo>();
    recordInfo->name = aType->name;
    recordInfo->type = aType;
    recordInfo->typeHash = GetRecordTypeHash(aType);
    recordInfo->shortName = GetRecordShortName<std::string>(aType->name);

    if (const auto parentInfo = GetRecordInfo(aType->parent))
    {
        recordInfo->parent = aType->parent;
        recordInfo->props.insert(parentInfo->props.begin(), parentInfo->props.end());
    }

    for (uint32_t funcIndex = 0u; funcIndex < aType->funcs.Size(); ++funcIndex)
    {
        const auto func = aType->funcs[funcIndex];

        auto propName = ResolvePropertyName(aType, func->shortName);

        const auto propInfo = MakeInstance<TweakDBPropertyInfo>();
        propInfo->name = CName(propName.c_str());

        // Case: Foreign Key Array => TweakDBID[]
        if (!func->returnType)
        {
            propInfo->type = TypeLocator<ERTDBFlatType::TweakDBIDArray>::Get();
            propInfo->isArray = true;
            propInfo->elementType = TypeLocator<ERTDBFlatType::TweakDBID>::Get();
            propInfo->isForeignKey = true;
            propInfo->foreignType = GetInnerType<CClass>(func->params[0]->type);

            // Skip related functions:
            // func Get[Prop]Count()
            // func Get[Prop]Item()
            // func Get[Prop]ItemHandle()
            // func [Prop]Contains()
            funcIndex += 4;
        }
        else
        {
            switch (auto returnType = func->returnType->type; returnType->GetType())
            {
            case ERTTIType::WeakHandle:
            {
                // Case: Foreign Key => TweakDBID
                propInfo->type = TypeLocator<ERTDBFlatType::TweakDBID>::Get();
                propInfo->isForeignKey = true;
                propInfo->foreignType = GetInnerType<CClass>(returnType);

                // Skip related function:
                // func Get[Prop]Handle()
                funcIndex += 1;
                break;
            }
            case ERTTIType::Array:
            {
                if (IsResRefTokenArray(returnType))
                {
                    propInfo->type = TypeLocator<ERTDBFlatType::ResRefArray>::Get();
                    propInfo->isArray = true;
                    propInfo->elementType = TypeLocator<ERTDBFlatType::ResRef>::Get();

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    funcIndex += 2;
                }
                else
                {
                    propInfo->type = returnType;
                    propInfo->isArray = true;
                    propInfo->elementType = GetInnerType(returnType);

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    // func [Prop]Contains()
                    funcIndex += 3;
                }
                break;
            }
            case ERTTIType::Enum:
            {
                // Some types have additional enum getters,
                // but they're not backed by any flat.
                continue;
            }
            default:
            {
                if (IsResRefToken(returnType))
                {
                    propInfo->type = TypeLocator<ERTDBFlatType::ResRef>::Get();
                }
                else if (returnType->GetType() == ERTTIType::Name)
                {
                    // Getter for LocKey returns CName, so we have to get the actual property type from the flat value.
                    // If a flat doesn't exist, assume the property type.
                    if (const auto flat = m_tweakDb->GetFlatValue(GetRTDBFlatID(aType->name, propName)))
                        propInfo->type = flat->GetValue().type;
                    else
                        propInfo->type = returnType;
                }
                else
                {
                    propInfo->type = returnType;
                }
            }
            }
        }

        assert(propInfo->type);

        propInfo->appendix = PropSeparator;
        propInfo->appendix.append(propName);

        recordInfo->props[propInfo->name] = propInfo;
    }

    {
        if (const auto extraFlatsIt = s_extraFlats.find(aType->name); extraFlatsIt != s_extraFlats.end())
        {
            for (const auto& extraFlat : extraFlatsIt.value())
            {
                const auto propInfo = MakeInstance<TweakDBPropertyInfo>();
                propInfo->name = CName(extraFlat.appendix.c_str() + 1);
                propInfo->appendix = extraFlat.appendix;
                propInfo->type = m_rtti->GetType(extraFlat.typeName);

                if (propInfo->type->GetType() == ERTTIType::Array)
                {
                    const auto arrayType = reinterpret_cast<const CRTTIArrayType*>(propInfo->type);
                    propInfo->elementType = arrayType->innerType;
                    propInfo->isArray = true;
                }

                if (!extraFlat.foreignTypeName.IsNone())
                {
                    propInfo->foreignType = m_rtti->GetClass(extraFlat.foreignTypeName);
                    propInfo->isForeignKey = true;
                }

                propInfo->isExtra = true;
                recordInfo->props[propInfo->name] = propInfo;
            }
        }
    }

    for (const auto& propInfo : recordInfo->props | std::views::values)
    {
        if (!propInfo->isExtra)
        {
            propInfo->defaultValue = ResolveDefaultValue(aType, propInfo->appendix);
        }
    }

    {
        std::unique_lock lockRW(m_mutex);
        m_resolved.insert({recordInfo->name, recordInfo});
    }

    return recordInfo;
}

std::string TweakDBReflection::ResolvePropertyName(const CClass* aClass, CName aGetterName) const
{
    std::string funcName = aGetterName.ToString();
    std::string propName = Decapitalize(funcName);

    const auto propId = GetRTDBFlatID(aClass->GetName(), propName);

    std::shared_lock flatLockR(m_tweakDb->mutex00);

    if (const auto propFlat = m_tweakDb->flats.Find(propId); propFlat == m_tweakDb->flats.End())
    {
        return funcName;
    }
    return propName;
}

int32_t TweakDBReflection::ResolveDefaultValue(const CClass* aType, const std::string& aPropName) const
{
    std::string defaultFlatName = TweakSource::SchemaPackage;
    defaultFlatName.append(NameSeparator);
    defaultFlatName.append(GetRecordShortName<std::string>(aType->GetName()));

    if (!aPropName.starts_with(NameSeparator))
    {
        defaultFlatName.append(NameSeparator);
    }

    defaultFlatName.append(aPropName);

    const auto defaultFlatId = TweakDBID(defaultFlatName);

    std::shared_lock flatLockR(m_tweakDb->mutex00);

    const auto defaultFlat = m_tweakDb->flats.Find(defaultFlatId);

    if (defaultFlat == m_tweakDb->flats.End())
        return -1;

    return defaultFlat->ToTDBOffset();
}

void TweakDBReflection::RegisterExtraFlat(CName aRecordType, const std::string& aPropName, CName aPropType,
                                          CName aForeignType)
{
    s_extraFlats[aRecordType].push_back({aPropType, aForeignType, NameSeparator + aPropName});
}

void TweakDBReflection::RegisterDescendants(const TweakDBID aParentId, const Core::Set<TweakDBID>& aDescendantIds)
{
    s_descendantMap[aParentId].insert(aDescendantIds.begin(), aDescendantIds.end());

    for (const auto& descendantId : aDescendantIds)
    {
        s_parentMap[descendantId] = aParentId;
    }
}

bool TweakDBReflection::IsOriginalRecord(const TweakDBID aRecordId) const
{
    return s_parentMap.contains(aRecordId);
}

bool TweakDBReflection::IsOriginalBaseRecord(const TweakDBID aParentId) const
{
    return s_descendantMap.contains(aParentId);
}

TweakDBID TweakDBReflection::GetOriginalParent(const TweakDBID aRecordId) const
{
    return s_parentMap[aRecordId];
}

const Core::Set<TweakDBID>& TweakDBReflection::GetOriginalDescendants(const TweakDBID aSourceId) const
{
    return s_descendantMap[aSourceId];
}

std::string TweakDBReflection::ToString(TweakDBID aID) const
{
    CString str;
    CallStatic("gamedataTDBIDHelper", "ToStringDEBUG", str, aID);
    return {str.c_str(), str.Length()};
}

TweakDB* TweakDBReflection::GetTweakDB() const
{
    return m_tweakDb;
}

} // namespace Red
