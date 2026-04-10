#include "Reflection.hpp"
#include "Red/TweakDB/Alias.hpp"
#include "Red/TweakDB/Source/Grammar.hpp"
#include "Red/TweakDB/Source/Source.hpp"

namespace
{
constexpr auto BaseRecordTypeName = Red::GetTypeName<Red::TweakDBRecord>();

constexpr auto ResRefTypeName = Red::GetTypeName<Red::RaRef<Red::CResource>>();
constexpr auto ResRefArrayTypeName = Red::GetTypeName<Red::DynArray<Red::RaRef<Red::CResource>>>();

constexpr auto ScriptResRefTypeName = Red::GetTypeName<Red::ResRef>();
constexpr auto ScriptResRefArrayTypeName = Red::GetTypeName<Red::DynArray<Red::ResRef>>();

constexpr auto NameSeparator = Red::TweakGrammar::Name::Separator;
constexpr auto PropSeparator = std::string_view(NameSeparator);
constexpr auto DataOffsetSize = 12;
} // namespace

Red::TweakDBReflection::TweakDBReflection(TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
{
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::GetRecordInfo(const CClass* aType)
{
    return CollectRecordInfo(aType);
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::GetRecordInfo(const CName& aTypeName)
{
    if (const auto* type = GetRTTI()->GetClass(aTypeName))
        return GetRecordInfo(type);
    return nullptr;
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::FindRecordInfo(const CClass* aType) const
{
    std::shared_lock lockR(m_mutex);
    if (const auto iter = m_resolved.find(aType->GetName()); iter != m_resolved.end())
        return iter->second.get();
    return nullptr;
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::FindRecordInfo(const CName& aTypeName) const
{
    if (const auto* type = GetRTTI()->GetClass(aTypeName))
        return FindRecordInfo(type);
    return nullptr;
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::CollectRecordInfo(const CClass* aType, const TweakDBID aSampleId)
{
    if (!IsRecordType(aType))
        return nullptr;

    if (const auto* existing = FindRecordInfo(aType))
        return existing;

    auto sampleId = aSampleId;
    if (!sampleId.IsValid())
    {
        sampleId = GetRecordSampleId(aType);

        if (!sampleId.IsValid())
            return nullptr;
    }

    const auto recordInfo = Red::MakeInstance<TweakDBRecordInfo>();
    recordInfo->SetName(aType->name);
    recordInfo->SetType(aType);

    if (const auto* parentInfo = CollectRecordInfo(aType->parent, sampleId))
    {
        *recordInfo += *parentInfo;
    }

    const auto baseOffset = aType->parent->size;

    for (uint32_t funcIndex = 0u; funcIndex < aType->funcs.Size(); ++funcIndex)
    {
        const auto func = aType->funcs[funcIndex];

        auto propName = ResolvePropertyName(sampleId, func->shortName);

        const auto propInfo = Red::MakeInstance<TweakDBPropertyInfo>();
        propInfo->SetName(propName);

        propInfo->SetDataOffset(baseOffset + (recordInfo->GetProperties().size() * DataOffsetSize));

        // Case: Foreign Key Array => TweakDBID[]
        if (!func->returnType)
        {
            const auto arrayType = reinterpret_cast<CRTTIArrayType*>(func->params[0]->type);
            const auto handleType = reinterpret_cast<CRTTIWeakHandleType*>(arrayType->innerType);
            const auto recordType = reinterpret_cast<CClass*>(handleType->innerType);

            propInfo->SetType(GetRTTI()->GetType(ERTDBFlatType::TweakDBIDArray));
            propInfo->SetArray();
            propInfo->SetElementType(GetRTTI()->GetType(ERTDBFlatType::TweakDBID));
            propInfo->SetForeignKey();
            propInfo->SetForeignType(recordType);

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
                const auto handleType = reinterpret_cast<CRTTIWeakHandleType*>(returnType);
                const auto recordType = reinterpret_cast<CClass*>(handleType->innerType);

                propInfo->SetType(GetRTTI()->GetType(ERTDBFlatType::TweakDBID));
                propInfo->SetForeignKey();
                propInfo->SetForeignType(recordType);

                // Skip related function:
                // func Get[Prop]Handle()
                funcIndex += 1;
                break;
            }
            case ERTTIType::Array:
            {
                if (IsResRefTokenArray(returnType))
                {
                    propInfo->SetType(GetRTTI()->GetType(ERTDBFlatType::ResRefArray));
                    propInfo->SetArray();
                    propInfo->SetElementType(GetRTTI()->GetType(ERTDBFlatType::ResRef));

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    funcIndex += 2;
                }
                else
                {
                    const auto arrayType = reinterpret_cast<CRTTIArrayType*>(returnType);
                    const auto elementType = arrayType->innerType;

                    propInfo->SetType(returnType);
                    propInfo->SetArray();
                    propInfo->SetElementType(elementType);

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
                    propInfo->SetType(GetRTTI()->GetType(ERTDBFlatType::ResRef));
                }
                else
                {
                    // Getter for LocKey returns CName, so we have to get
                    // the actual property type from the flat value.
                    if (returnType->GetType() == ERTTIType::Name)
                    {
                        const auto propId = sampleId + PropSeparator + propName;
                        const auto flat = m_tweakDb->GetFlatValue(propId);
                        returnType = flat->GetValue().type;
                    }

                    propInfo->SetType(returnType);
                }
            }
            }
        }

        if (propInfo->GetDataOffset())
        {
            propInfo->SetDefaultValue(ResolveDefaultValue(aType, propInfo->GetAppendix()));
        }

        assert(propInfo->IsValid());

        recordInfo->AddProperty(propInfo);
    }

    {
        if (const auto extraFlatsIt = s_extraFlats.find(aType->name); extraFlatsIt != s_extraFlats.end())
        {
            for (const auto& [typeName, foreignTypeName, appendix] : extraFlatsIt.value())
            {
                const auto propInfo = Red::MakeInstance<TweakDBPropertyInfo>();
                propInfo->SetName(appendix.c_str() + 1);
                propInfo->SetType(GetRTTI()->GetType(typeName));

                if (propInfo->GetType()->GetType() == ERTTIType::Array)
                {
                    const auto arrayType = reinterpret_cast<const CRTTIArrayType*>(propInfo->GetType());
                    propInfo->SetElementType(arrayType->innerType);
                    propInfo->SetArray();
                }

                if (!foreignTypeName.IsNone())
                {
                    propInfo->SetForeignType(GetRTTI()->GetClass(foreignTypeName));
                    propInfo->SetForeignKey();
                }

                assert(recordInfo->AddProperty(propInfo) && "Failed to add extra flat property");
            }
        }
    }

    const auto r = RegisterRecordInfo(recordInfo);
    assert(r && "Failed to register record info");
    return r.get();
}

Red::TweakDBID Red::TweakDBReflection::GetRecordSampleId(const CClass* aType) const
{
    std::shared_lock recordLockR(m_tweakDb->mutex01);
    auto* records = m_tweakDb->recordsByType.Get(const_cast<CClass*>(aType));

    if (records == nullptr)
        return {};

    return records->Begin()->GetPtr<TweakDBRecord>()->recordID;
}

uint32_t Red::TweakDBReflection::GetRecordTypeHash(const std::string& aName)
{
    return Murmur3_32(reinterpret_cast<const uint8_t*>(aName.c_str()), aName.length());
}

std::string Red::TweakDBReflection::ResolvePropertyName(const TweakDBID aSampleId, const CName& aGetterName) const
{
    std::string propName = aGetterName.ToString();
    propName[0] = static_cast<char>(std::tolower(propName[0]));

    const auto propId = aSampleId + PropSeparator + propName;

    std::shared_lock flatLockR(m_tweakDb->mutex00);

    if (const auto propFlat = m_tweakDb->flats.Find(propId); propFlat == m_tweakDb->flats.End())
        propName[0] = static_cast<char>(std::toupper(propName[0]));

    return propName;
}

int32_t Red::TweakDBReflection::ResolveDefaultValue(const CClass* aType, const std::string& aPropName) const
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

const Red::CBaseRTTIType* Red::TweakDBReflection::GetFlatType(const CName& aTypeName)
{
    const CBaseRTTIType* type = GetRTTI()->GetType(aTypeName);

    if (!IsFlatType(type))
        return nullptr;

    return type;
}

const Red::CClass* Red::TweakDBReflection::GetRecordType(const CName& aTypeName)
{
    return GetRecordType(aTypeName.ToString());
}

const Red::CClass* Red::TweakDBReflection::GetRecordType(const char* aTypeName)
{
    const auto aFullName = GetRecordFullName<CName>(aTypeName);

    const CClass* type = GetRTTI()->GetClass(aFullName);

    if (!IsRecordType(type))
        return nullptr;

    return type;
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetArrayType(const CName& aTypeName)
{
    return GetRTTI()->GetType(GetArrayTypeName(aTypeName));
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetArrayType(const CBaseRTTIType* aType)
{
    return GetRTTI()->GetType(GetArrayTypeName(aType));
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetElementType(const CName& aTypeName)
{
    return GetElementType(GetRTTI()->GetType(aTypeName));
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetElementType(const CBaseRTTIType* aType)
{
    if (!aType || aType->GetType() != ERTTIType::Array)
        return nullptr;

    return reinterpret_cast<const CRTTIBaseArrayType*>(aType)->innerType;
}

bool Red::TweakDBReflection::IsFlatType(const CName& aTypeName)
{
    switch (aTypeName)
    {
    case ERTDBFlatType::Int:
    case ERTDBFlatType::Float:
    case ERTDBFlatType::Bool:
    case ERTDBFlatType::String:
    case ERTDBFlatType::CName:
    case ERTDBFlatType::LocKey:
    case ERTDBFlatType::ResRef:
    case ERTDBFlatType::TweakDBID:
    case ERTDBFlatType::Quaternion:
    case ERTDBFlatType::EulerAngles:
    case ERTDBFlatType::Vector3:
    case ERTDBFlatType::Vector2:
    case ERTDBFlatType::Color:
    case ERTDBFlatType::IntArray:
    case ERTDBFlatType::FloatArray:
    case ERTDBFlatType::BoolArray:
    case ERTDBFlatType::StringArray:
    case ERTDBFlatType::CNameArray:
    case ERTDBFlatType::LocKeyArray:
    case ERTDBFlatType::ResRefArray:
    case ERTDBFlatType::TweakDBIDArray:
    case ERTDBFlatType::QuaternionArray:
    case ERTDBFlatType::EulerAnglesArray:
    case ERTDBFlatType::Vector3Array:
    case ERTDBFlatType::Vector2Array:
    case ERTDBFlatType::ColorArray:
        return true;
    default:
        return false;
    }
}

bool Red::TweakDBReflection::IsFlatType(const CBaseRTTIType* aType)
{
    return aType && IsFlatType(aType->GetName());
}

bool Red::TweakDBReflection::IsRecordType(const CName& aTypeName)
{
    return aTypeName && IsRecordType(CRTTISystem::Get()->GetClass(aTypeName));
}

bool Red::TweakDBReflection::IsRecordType(const CClass* aType)
{
    static CBaseRTTIType* s_baseRecordType = CRTTISystem::Get()->GetClass(BaseRecordTypeName);

    return aType && aType != s_baseRecordType && aType->IsA(s_baseRecordType);
}

bool Red::TweakDBReflection::IsArrayType(const CName& aTypeName)
{
    switch (aTypeName)
    {
    case ERTDBFlatType::IntArray:
    case ERTDBFlatType::FloatArray:
    case ERTDBFlatType::BoolArray:
    case ERTDBFlatType::StringArray:
    case ERTDBFlatType::CNameArray:
    case ERTDBFlatType::LocKeyArray:
    case ERTDBFlatType::ResRefArray:
    case ERTDBFlatType::TweakDBIDArray:
    case ERTDBFlatType::QuaternionArray:
    case ERTDBFlatType::EulerAnglesArray:
    case ERTDBFlatType::Vector3Array:
    case ERTDBFlatType::Vector2Array:
    case ERTDBFlatType::ColorArray:
        return true;
    default:
        return false;
    }
}

bool Red::TweakDBReflection::IsArrayType(const CBaseRTTIType* aType)
{
    return aType && IsArrayType(aType->GetName());
}

bool Red::TweakDBReflection::IsForeignKey(const CName& aTypeName)
{
    return aTypeName == ERTDBFlatType::TweakDBID;
}

bool Red::TweakDBReflection::IsForeignKey(const CBaseRTTIType* aType)
{
    return aType && IsForeignKey(aType->GetName());
}

bool Red::TweakDBReflection::IsForeignKeyArray(const CName& aTypeName)
{
    return aTypeName == ERTDBFlatType::TweakDBIDArray;
}

bool Red::TweakDBReflection::IsForeignKeyArray(const CBaseRTTIType* aType)
{
    return aType && IsForeignKeyArray(aType->GetName());
}

bool Red::TweakDBReflection::IsResRefToken(const CName& aTypeName)
{
    return aTypeName == ScriptResRefTypeName || aTypeName == ResRefTypeName;
}

bool Red::TweakDBReflection::IsResRefToken(const CBaseRTTIType* aType)
{
    return aType && IsResRefToken(aType->GetName());
}

bool Red::TweakDBReflection::IsResRefTokenArray(const CName& aTypeName)
{
    return aTypeName == ScriptResRefArrayTypeName || aTypeName == ResRefArrayTypeName;
}

bool Red::TweakDBReflection::IsResRefTokenArray(const CBaseRTTIType* aType)
{
    return aType && IsResRefTokenArray(aType->GetName());
}

Red::CName Red::TweakDBReflection::GetArrayTypeName(const CName& aTypeName)
{
    switch (aTypeName)
    {
    case ERTDBFlatType::Int:
        return ERTDBFlatType::IntArray;
    case ERTDBFlatType::Float:
        return ERTDBFlatType::FloatArray;
    case ERTDBFlatType::Bool:
        return ERTDBFlatType::BoolArray;
    case ERTDBFlatType::String:
        return ERTDBFlatType::StringArray;
    case ERTDBFlatType::CName:
        return ERTDBFlatType::CNameArray;
    case ERTDBFlatType::LocKey:
        return ERTDBFlatType::LocKeyArray;
    case ERTDBFlatType::ResRef:
        return ERTDBFlatType::ResRefArray;
    case ERTDBFlatType::TweakDBID:
        return ERTDBFlatType::TweakDBIDArray;
    case ERTDBFlatType::Quaternion:
        return ERTDBFlatType::QuaternionArray;
    case ERTDBFlatType::EulerAngles:
        return ERTDBFlatType::EulerAnglesArray;
    case ERTDBFlatType::Vector3:
        return ERTDBFlatType::Vector3Array;
    case ERTDBFlatType::Vector2:
        return ERTDBFlatType::Vector2Array;
    case ERTDBFlatType::Color:
        return ERTDBFlatType::ColorArray;
    default:
        return {};
    }
}

Red::CName Red::TweakDBReflection::GetArrayTypeName(const CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetArrayTypeName(aType->GetName());
}

Red::CName Red::TweakDBReflection::GetElementTypeName(const CName& aTypeName)
{
    switch (aTypeName)
    {
    case ERTDBFlatType::IntArray:
        return ERTDBFlatType::Int;
    case ERTDBFlatType::FloatArray:
        return ERTDBFlatType::Float;
    case ERTDBFlatType::BoolArray:
        return ERTDBFlatType::Bool;
    case ERTDBFlatType::StringArray:
        return ERTDBFlatType::String;
    case ERTDBFlatType::CNameArray:
        return ERTDBFlatType::CName;
    case ERTDBFlatType::TweakDBIDArray:
        return ERTDBFlatType::TweakDBID;
    case ERTDBFlatType::LocKeyArray:
        return ERTDBFlatType::LocKey;
    case ERTDBFlatType::ResRefArray:
        return ERTDBFlatType::ResRef;
    case ERTDBFlatType::QuaternionArray:
        return ERTDBFlatType::Quaternion;
    case ERTDBFlatType::EulerAnglesArray:
        return ERTDBFlatType::EulerAngles;
    case ERTDBFlatType::Vector3Array:
        return ERTDBFlatType::Vector3;
    case ERTDBFlatType::Vector2Array:
        return ERTDBFlatType::Vector2;
    case ERTDBFlatType::ColorArray:
        return ERTDBFlatType::Color;
    default:
        return {};
    }
}

Red::CName Red::TweakDBReflection::GetElementTypeName(const CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetElementTypeName(aType->GetName());
}

Red::InstancePtr<> Red::TweakDBReflection::Construct(const CName& aTypeName)
{
    switch (aTypeName)
    {
    case ERTDBFlatType::Int:
        return MakeInstance<int>();
    case ERTDBFlatType::Float:
        return MakeInstance<float>();
    case ERTDBFlatType::Bool:
        return MakeInstance<bool>();
    case ERTDBFlatType::String:
        return MakeInstance<CString>();
    case ERTDBFlatType::CName:
        return MakeInstance<CName>();
    case ERTDBFlatType::LocKey:
        return MakeInstance<LocKeyWrapper>();
    case ERTDBFlatType::ResRef:
        return MakeInstance<ResourceAsyncReference<>>();
    case ERTDBFlatType::TweakDBID:
        return MakeInstance<TweakDBID>();
    case ERTDBFlatType::Quaternion:
        return MakeInstance<Quaternion>();
    case ERTDBFlatType::EulerAngles:
        return MakeInstance<EulerAngles>();
    case ERTDBFlatType::Vector3:
        return MakeInstance<Vector3>();
    case ERTDBFlatType::Vector2:
        return MakeInstance<Vector2>();
    case ERTDBFlatType::Color:
        return MakeInstance<Color>();
    case ERTDBFlatType::IntArray:
        return MakeInstance<DynArray<int>>();
    case ERTDBFlatType::FloatArray:
        return MakeInstance<DynArray<float>>();
    case ERTDBFlatType::BoolArray:
        return MakeInstance<DynArray<bool>>();
    case ERTDBFlatType::StringArray:
        return MakeInstance<DynArray<CString>>();
    case ERTDBFlatType::CNameArray:
        return MakeInstance<DynArray<CName>>();
    case ERTDBFlatType::LocKeyArray:
        return MakeInstance<DynArray<LocKeyWrapper>>();
    case ERTDBFlatType::ResRefArray:
        return MakeInstance<DynArray<ResourceAsyncReference<>>>();
    case ERTDBFlatType::TweakDBIDArray:
        return MakeInstance<DynArray<TweakDBID>>();
    case ERTDBFlatType::QuaternionArray:
        return MakeInstance<DynArray<Quaternion>>();
    case ERTDBFlatType::EulerAnglesArray:
        return MakeInstance<DynArray<EulerAngles>>();
    case ERTDBFlatType::Vector3Array:
        return MakeInstance<DynArray<Vector3>>();
    case ERTDBFlatType::Vector2Array:
        return MakeInstance<DynArray<Vector2>>();
    case ERTDBFlatType::ColorArray:
        return MakeInstance<DynArray<Color>>();
    default:
        return {};
    }
}

Red::InstancePtr<> Red::TweakDBReflection::Construct(const CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return Construct(aType->GetName());
}

void Red::TweakDBReflection::RegisterExtraFlat(const CName& aRecordType, const std::string& aPropName,
                                               const CName& aPropType, const CName& aForeignType)
{
    s_extraFlats[aRecordType].push_back({aPropType, aForeignType, NameSeparator + aPropName});
}

void Red::TweakDBReflection::RegisterDescendants(const TweakDBID aParentId, const Core::Set<TweakDBID>& aDescendantIds)
{
    s_descendantMap[aParentId].insert(aDescendantIds.begin(), aDescendantIds.end());

    for (const auto& descendantId : aDescendantIds)
    {
        s_parentMap[descendantId] = aParentId;
    }
}

bool Red::TweakDBReflection::IsOriginalRecord(const TweakDBID aRecordId)
{
    return s_parentMap.contains(aRecordId);
}

bool Red::TweakDBReflection::IsOriginalBaseRecord(const TweakDBID aParentId)
{
    return s_descendantMap.contains(aParentId);
}

Red::TweakDBID Red::TweakDBReflection::GetOriginalParent(const TweakDBID aRecordId)
{
    return s_parentMap[aRecordId];
}

const Core::Set<Red::TweakDBID>& Red::TweakDBReflection::GetOriginalDescendants(Red::TweakDBID aSourceId)
{
    return s_descendantMap[aSourceId];
}

std::string Red::TweakDBReflection::ToString(Red::TweakDBID aID)
{
    Red::CString str;
    Red::CallStatic("gamedataTDBIDHelper", "ToStringDEBUG", str, aID);
    return {str.c_str(), str.Length()};
}

Red::TweakDB* Red::TweakDBReflection::GetTweakDB() const
{
    return m_tweakDb;
}

Red::IRTTISystem* Red::TweakDBReflection::GetRTTI()
{
    if (!s_rtti)
        s_rtti = CRTTISystem::Get();
    ;
    return s_rtti;
}

Core::SharedPtr<const Red::TweakDBRecordInfo> Red::TweakDBReflection::RegisterRecordInfo(
    const Core::SharedPtr<TweakDBRecordInfo>& aRecordInfo)
{
    std::unique_lock lockRW(m_mutex);
    if (auto [it, success] = m_resolved.insert({aRecordInfo->GetName(), aRecordInfo}); success)
    {
        return it->second;
    }
    return nullptr;
}
