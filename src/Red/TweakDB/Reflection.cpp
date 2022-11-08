#include "Reflection.hpp"
#include "Types.hpp"

#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/Scripting/CProperty.hpp>

namespace
{
constexpr size_t DataOffsetSize = 3;
}

Red::TweakDB::Reflection::Reflection()
    : Reflection(Instance::Get())
{
}

Red::TweakDB::Reflection::Reflection(Instance* aTweakDb)
    : m_rtti(Red::CRTTISystem::Get())
    , m_tweakDb(aTweakDb)
{
}

const Red::CBaseRTTIType* Red::TweakDB::Reflection::GetFlatType(Red::CName aTypeName)
{
    const Red::CBaseRTTIType* type = m_rtti->GetType(aTypeName);

    if (!IsFlatType(type))
        return nullptr;

    return type;
}

const Red::CClass* Red::TweakDB::Reflection::GetRecordType(Red::CName aTypeName)
{
    return GetRecordType(aTypeName.ToString());
}

const Red::CClass* Red::TweakDB::Reflection::GetRecordType(const char* aTypeName)
{
    auto aFullName = GetRecordFullName(aTypeName);

    Red::CClass* type = m_rtti->GetClass(aFullName);

    if (!IsRecordType(type))
        return nullptr;

    return type;
}

const Red::TweakDB::RecordTypeInfo* Red::TweakDB::Reflection::GetRecordInfo(const Red::CClass* aType)
{
    if (!IsRecordType(aType))
        return nullptr;

    auto iter = m_data.find(aType->GetName());

    if (iter != m_data.end())
        return iter->second.get();

    return CollectRecordInfo(aType).get();
}

const Red::TweakDB::RecordTypeInfo* Red::TweakDB::Reflection::GetRecordInfo(Red::CName aTypeName)
{
    auto iter = m_data.find(aTypeName);

    if (iter != m_data.end())
        return iter->second.get();

    return CollectRecordInfo(m_rtti->GetClass(aTypeName)).get();
}

Core::SharedPtr<Red::TweakDB::RecordTypeInfo> Red::TweakDB::Reflection::CollectRecordInfo(const Red::CClass* aType,
                                                                                        Red::TweakDBID aSampleId)
{
    if (!IsRecordType(aType))
        return nullptr;

    auto sampleId = aSampleId;
    if (!sampleId.IsValid())
    {
        sampleId = GetRecordSampleId(aType);

        if (!sampleId.IsValid())
            return nullptr;
    }

    auto recordInfo = Core::MakeShared<RecordTypeInfo>();
    recordInfo->name = aType->name;
    recordInfo->type = aType;
    recordInfo->typeHash = GetRecordTypeHash(aType);
    recordInfo->shortName = GetRecordShortName(aType->name);

    const auto parentInfo = CollectRecordInfo(aType->parent, sampleId);
    if (parentInfo)
    {
        recordInfo->parent = aType->parent;
        recordInfo->props.insert(parentInfo->props.begin(), parentInfo->props.end());
    }

    const auto baseOffset = aType->parent->size;

    for (uint32_t funcIndex = 0u; funcIndex < aType->funcs.size; ++funcIndex)
    {
        const auto func = aType->funcs[funcIndex];

        auto propName = ResolvePropertyName(sampleId, func->shortName);

        auto propInfo = Core::MakeShared<PropertyInfo>();
        propInfo->name = Red::CName(propName.c_str());
        propInfo->offset = baseOffset + DataOffsetSize * recordInfo->props.size();

        // Case: Foreign Key Array => TweakDBID[]
        if (!func->returnType)
        {
            const auto arrayType = reinterpret_cast<Red::CRTTIArrayType*>(func->params[0]->type);
            const auto handleType = reinterpret_cast<Red::CRTTIWeakHandleType*>(arrayType->innerType);
            const auto recordType = reinterpret_cast<Red::CClass*>(handleType->innerType);

            propInfo->type = m_rtti->GetType(EFlatType::TweakDBIDArray);
            propInfo->isArray = true;
            propInfo->elementType = m_rtti->GetType(EFlatType::TweakDBID);
            propInfo->isForeignKey = true;
            propInfo->foreignType = recordType;

            // Skip related functions:
            // func Get[Prop]Count()
            // func Get[Prop]Item()
            // func Get[Prop]ItemHandle()
            // func [Prop]Contains()
            funcIndex += 4;
        }
        else
        {
            auto returnType = func->returnType->type;

            switch (returnType->GetType())
            {
            case Red::ERTTIType::WeakHandle:
            {
                // Case: Foreign Key => TweakDBID
                const auto handleType = reinterpret_cast<Red::CRTTIWeakHandleType*>(returnType);
                const auto recordType = reinterpret_cast<Red::CClass*>(handleType->innerType);

                propInfo->type = m_rtti->GetType(EFlatType::TweakDBID);
                propInfo->isForeignKey = true;
                propInfo->foreignType = recordType;

                // Skip related function:
                // func Get[Prop]Handle()
                funcIndex += 1;
                break;
            }
            case Red::ERTTIType::Array:
            {
                if (IsResRefTokenArray(returnType))
                {
                    propInfo->type = m_rtti->GetType(Red::TweakDB::EFlatType::ResourceArray);
                    propInfo->isArray = true;
                    propInfo->elementType = m_rtti->GetType(EFlatType::Resource);

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    funcIndex += 2;
                }
                else
                {
                    const auto arrayType = reinterpret_cast<Red::CRTTIArrayType*>(returnType);
                    const auto elementType = reinterpret_cast<Red::CBaseRTTIType*>(arrayType->innerType);

                    propInfo->type = returnType;
                    propInfo->isArray = true;
                    propInfo->elementType = elementType;

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    // func [Prop]Contains()
                    funcIndex += 3;
                }
                break;
            }
            case Red::ERTTIType::Enum:
            {
                // Some types have additional enum getters,
                // but they're not backed by any flat.
                continue;
            }
            default:
            {
                if (IsResRefToken(returnType))
                {
                    propInfo->type = m_rtti->GetType(Red::TweakDB::EFlatType::Resource);
                }
                else
                {
                    // Getter for LocKey returns CName, so we have to get
                    // the actual property type from the flat value.
                    if (returnType->GetType() == Red::ERTTIType::Name)
                    {
                        static std::string dot = ".";
                        auto propId = sampleId + dot + propName;
                        auto flat = m_tweakDb->GetFlatValue(propId);
                        returnType = flat->GetValue().type;
                    }

                    propInfo->type = returnType;
                }
            }
            }
        }

        assert(propInfo->type);

        propInfo->appendix = ".";
        propInfo->appendix.append(propName);

        recordInfo->props.insert({ propInfo->name, propInfo });
    }

    m_data.insert({ recordInfo->name, recordInfo });

    return recordInfo;
}

Red::TweakDBID Red::TweakDB::Reflection::GetRecordSampleId(const Red::CClass* aType)
{
    std::shared_lock<Red::SharedMutex> recordLockR(m_tweakDb->mutex01);
    auto* records = m_tweakDb->recordsByType.Get(const_cast<Red::CClass*>(aType));

    if (records == nullptr)
        return {};

    return records->Begin()->GetPtr<Red::gamedataTweakDBRecord>()->recordID;
}

uint32_t Red::TweakDB::Reflection::GetRecordTypeHash(const Red::CClass* aType)
{
    std::shared_lock<Red::SharedMutex> recordLockR(m_tweakDb->mutex01);
    auto* records = m_tweakDb->recordsByType.Get(const_cast<Red::CClass*>(aType));

    if (records == nullptr)
        return 0;

    return records->Begin()->GetPtr<Red::gamedataTweakDBRecord>()->GetTweakBaseHash();
}

std::string Red::TweakDB::Reflection::ResolvePropertyName(Red::TweakDBID aSampleId, Red::CName aGetterName)
{
    static std::string dot = ".";

    std::string propName = aGetterName.ToString();
    propName[0] = static_cast<char>(std::tolower(propName[0]));

    auto propId = aSampleId + dot + propName;

    std::shared_lock<Red::SharedMutex> flatLockR(m_tweakDb->mutex00);

    auto flat = m_tweakDb->flats.Find(propId);

    flatLockR.unlock();

    if (flat == m_tweakDb->flats.End())
        propName[0] = static_cast<char>(std::toupper(propName[0]));

    return propName;
}
