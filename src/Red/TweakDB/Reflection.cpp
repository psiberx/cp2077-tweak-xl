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

Red::TweakDBReflection::TweakDBReflection(Red::TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
{
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::GetRecordInfo(const Red::CClass* aType)
{
    return CollectRecordInfo(aType);
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::GetRecordInfo(Red::CName aTypeName)
{
    if (auto* type = GetRTTI()->GetClass(aTypeName))
        return GetRecordInfo(type);
    return nullptr;
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::FindRecordInfo(const Red::CClass* aType)
{
    std::shared_lock lockR(m_mutex);
    if (auto iter = m_resolved.find(aType->GetName()); iter != m_resolved.end())
        return iter->second.get();
    return nullptr;
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::FindRecordInfo(Red::CName aTypeName)
{
    if (auto* type = GetRTTI()->GetClass(aTypeName))
        return FindRecordInfo(type);
    return nullptr;
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::CollectRecordInfo(const Red::CClass* aType,
                                                                        Red::TweakDBID aSampleId)
{
    if (!IsRecordType(aType))
        return nullptr;

    if (auto* existing = FindRecordInfo(aType))
        return existing;

    auto sampleId = aSampleId;
    if (!sampleId.IsValid())
    {
        sampleId = GetRecordSampleId(aType);

        if (!sampleId.IsValid())
            return nullptr;
    }

    auto recordInfo = Red::MakeInstance<Red::TweakDBRecordInfo>();
    recordInfo->SetName(aType->name);
    recordInfo->SetType(aType);

    const auto parentInfo = CollectRecordInfo(aType->parent, sampleId);
    if (parentInfo)
    {
        *recordInfo += *parentInfo;
    }

    const auto baseOffset = aType->parent->size;

    for (uint32_t funcIndex = 0u; funcIndex < aType->funcs.Size(); ++funcIndex)
    {
        const auto func = aType->funcs[funcIndex];

        auto propName = ResolvePropertyName(sampleId, func->shortName);

        auto propInfo = Red::MakeInstance<Red::TweakDBPropertyInfo>();
        propInfo->SetName(propName);

        propInfo->SetDataOffset(baseOffset + (recordInfo->GetProperties().size() * DataOffsetSize));

        // Case: Foreign Key Array => TweakDBID[]
        if (!func->returnType)
        {
            const auto arrayType = reinterpret_cast<Red::CRTTIArrayType*>(func->params[0]->type);
            const auto handleType = reinterpret_cast<Red::CRTTIWeakHandleType*>(arrayType->innerType);
            const auto recordType = reinterpret_cast<Red::CClass*>(handleType->innerType);

            propInfo->SetType(GetRTTI()->GetType(Red::ERTDBFlatType::TweakDBIDArray));
            propInfo->SetArray();
            propInfo->SetElementType(GetRTTI()->GetType(Red::ERTDBFlatType::TweakDBID));
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
            auto returnType = func->returnType->type;

            switch (returnType->GetType())
            {
            case Red::ERTTIType::WeakHandle:
            {
                // Case: Foreign Key => TweakDBID
                const auto handleType = reinterpret_cast<Red::CRTTIWeakHandleType*>(returnType);
                const auto recordType = reinterpret_cast<Red::CClass*>(handleType->innerType);

                propInfo->SetType(GetRTTI()->GetType(Red::ERTDBFlatType::TweakDBID));
                propInfo->SetForeignKey();
                propInfo->SetForeignType(recordType);

                // Skip related function:
                // func Get[Prop]Handle()
                funcIndex += 1;
                break;
            }
            case Red::ERTTIType::Array:
            {
                if (IsResRefTokenArray(returnType))
                {
                    propInfo->SetType(GetRTTI()->GetType(Red::ERTDBFlatType::ResRefArray));
                    propInfo->SetArray();
                    propInfo->SetElementType(GetRTTI()->GetType(Red::ERTDBFlatType::ResRef));

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    funcIndex += 2;
                }
                else
                {
                    const auto arrayType = reinterpret_cast<Red::CRTTIArrayType*>(returnType);
                    const auto elementType = reinterpret_cast<Red::CBaseRTTIType*>(arrayType->innerType);

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
                    propInfo->SetType(GetRTTI()->GetType(Red::ERTDBFlatType::ResRef));
                }
                else
                {
                    // Getter for LocKey returns CName, so we have to get
                    // the actual property type from the flat value.
                    if (returnType->GetType() == Red::ERTTIType::Name)
                    {
                        auto propId = sampleId + PropSeparator + propName;
                        auto flat = m_tweakDb->GetFlatValue(propId);
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
        auto extraFlatsIt = s_extraFlats.find(aType->name);
        if (extraFlatsIt != s_extraFlats.end())
        {
            for (const auto& extraFlat : extraFlatsIt.value())
            {
                auto propInfo = Red::MakeInstance<Red::TweakDBPropertyInfo>();
                propInfo->SetName(extraFlat.appendix.c_str() + 1);
                propInfo->SetType(GetRTTI()->GetType(extraFlat.typeName));

                if (propInfo->GetType()->GetType() == Red::ERTTIType::Array)
                {
                    const auto arrayType = reinterpret_cast<const Red::CRTTIArrayType*>(propInfo->GetType());
                    propInfo->SetElementType(arrayType->innerType);
                    propInfo->SetArray();
                }

                if (!extraFlat.foreignTypeName.IsNone())
                {
                    propInfo->SetForeignType(GetRTTI()->GetClass(extraFlat.foreignTypeName));
                    propInfo->SetForeignKey();
                }

                assert(recordInfo->AddProperty(propInfo) && "Failed to add extra flat property");
            }
        }
    }

    auto r = RegisterRecordInfo(recordInfo);
    assert(r && "Failed to register record info");
    return r.get();
}

Red::TweakDBID Red::TweakDBReflection::GetRecordSampleId(const Red::CClass* aType)
{
    std::shared_lock<Red::SharedSpinLock> recordLockR(m_tweakDb->mutex01);
    auto* records = m_tweakDb->recordsByType.Get(const_cast<Red::CClass*>(aType));

    if (records == nullptr)
        return {};

    return records->Begin()->GetPtr<Red::TweakDBRecord>()->recordID;
}

uint32_t Red::TweakDBReflection::GetRecordTypeHash(const std::string& aName)
{
    return Red::Murmur3_32(reinterpret_cast<const uint8_t*>(aName.c_str()), aName.length());
}

std::string Red::TweakDBReflection::ResolvePropertyName(Red::TweakDBID aSampleId, Red::CName aGetterName)
{
    std::string propName = aGetterName.ToString();
    propName[0] = static_cast<char>(std::tolower(propName[0]));

    auto propId = aSampleId + PropSeparator + propName;

    std::shared_lock<Red::SharedSpinLock> flatLockR(m_tweakDb->mutex00);

    auto propFlat = m_tweakDb->flats.Find(propId);
    if (propFlat == m_tweakDb->flats.End())
        propName[0] = static_cast<char>(std::toupper(propName[0]));

    return propName;
}

int32_t Red::TweakDBReflection::ResolveDefaultValue(const Red::CClass* aType, const std::string& aPropName)
{
    std::string defaultFlatName = TweakSource::SchemaPackage;
    defaultFlatName.append(NameSeparator);
    defaultFlatName.append(GetRecordShortName<std::string>(aType->GetName()));

    if (!aPropName.starts_with(NameSeparator))
    {
        defaultFlatName.append(NameSeparator);
    }

    defaultFlatName.append(aPropName);

    const auto defaultFlatId = Red::TweakDBID(defaultFlatName);

    std::shared_lock<Red::SharedSpinLock> flatLockR(m_tweakDb->mutex00);

    auto defaultFlat = m_tweakDb->flats.Find(defaultFlatId);

    if (defaultFlat == m_tweakDb->flats.End())
        return -1;

    return defaultFlat->ToTDBOffset();
}

const Red::CBaseRTTIType* Red::TweakDBReflection::GetFlatType(Red::CName aTypeName)
{
    const Red::CBaseRTTIType* type = GetRTTI()->GetType(aTypeName);

    if (!IsFlatType(type))
        return nullptr;

    return type;
}

const Red::CClass* Red::TweakDBReflection::GetRecordType(Red::CName aTypeName)
{
    return GetRecordType(aTypeName.ToString());
}

const Red::CClass* Red::TweakDBReflection::GetRecordType(const char* aTypeName)
{
    auto aFullName = GetRecordFullName<Red::CName>(aTypeName);

    Red::CClass* type = GetRTTI()->GetClass(aFullName);

    if (!IsRecordType(type))
        return nullptr;

    return type;
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetArrayType(Red::CName aTypeName)
{
    return GetRTTI()->GetType(GetArrayTypeName(aTypeName));
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetArrayType(const Red::CBaseRTTIType* aType)
{
    return GetRTTI()->GetType(GetArrayTypeName(aType));
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetElementType(Red::CName aTypeName)
{
    return GetElementType(GetRTTI()->GetType(aTypeName));
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetElementType(const Red::CBaseRTTIType* aType)
{
    if (!aType || aType->GetType() != Red::ERTTIType::Array)
        return nullptr;

    return reinterpret_cast<const Red::CRTTIBaseArrayType*>(aType)->innerType;
}

bool Red::TweakDBReflection::IsFlatType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case Red::ERTDBFlatType::Int:
    case Red::ERTDBFlatType::Float:
    case Red::ERTDBFlatType::Bool:
    case Red::ERTDBFlatType::String:
    case Red::ERTDBFlatType::CName:
    case Red::ERTDBFlatType::LocKey:
    case Red::ERTDBFlatType::ResRef:
    case Red::ERTDBFlatType::TweakDBID:
    case Red::ERTDBFlatType::Quaternion:
    case Red::ERTDBFlatType::EulerAngles:
    case Red::ERTDBFlatType::Vector3:
    case Red::ERTDBFlatType::Vector2:
    case Red::ERTDBFlatType::Color:
    case Red::ERTDBFlatType::IntArray:
    case Red::ERTDBFlatType::FloatArray:
    case Red::ERTDBFlatType::BoolArray:
    case Red::ERTDBFlatType::StringArray:
    case Red::ERTDBFlatType::CNameArray:
    case Red::ERTDBFlatType::LocKeyArray:
    case Red::ERTDBFlatType::ResRefArray:
    case Red::ERTDBFlatType::TweakDBIDArray:
    case Red::ERTDBFlatType::QuaternionArray:
    case Red::ERTDBFlatType::EulerAnglesArray:
    case Red::ERTDBFlatType::Vector3Array:
    case Red::ERTDBFlatType::Vector2Array:
    case Red::ERTDBFlatType::ColorArray:
        return true;
    default:
        return false;
    }
}

bool Red::TweakDBReflection::IsFlatType(const Red::CBaseRTTIType* aType)
{
    return aType && IsFlatType(aType->GetName());
}

bool Red::TweakDBReflection::IsRecordType(Red::CName aTypeName)
{
    return aTypeName && IsRecordType(Red::CRTTISystem::Get()->GetClass(aTypeName));
}

bool Red::TweakDBReflection::IsRecordType(const Red::CClass* aType)
{
    static Red::CBaseRTTIType* s_baseRecordType = Red::CRTTISystem::Get()->GetClass(BaseRecordTypeName);

    return aType && aType != s_baseRecordType && aType->IsA(s_baseRecordType);
}

bool Red::TweakDBReflection::IsArrayType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case Red::ERTDBFlatType::IntArray:
    case Red::ERTDBFlatType::FloatArray:
    case Red::ERTDBFlatType::BoolArray:
    case Red::ERTDBFlatType::StringArray:
    case Red::ERTDBFlatType::CNameArray:
    case Red::ERTDBFlatType::LocKeyArray:
    case Red::ERTDBFlatType::ResRefArray:
    case Red::ERTDBFlatType::TweakDBIDArray:
    case Red::ERTDBFlatType::QuaternionArray:
    case Red::ERTDBFlatType::EulerAnglesArray:
    case Red::ERTDBFlatType::Vector3Array:
    case Red::ERTDBFlatType::Vector2Array:
    case Red::ERTDBFlatType::ColorArray:
        return true;
    default:
        return false;
    }
}

bool Red::TweakDBReflection::IsArrayType(const Red::CBaseRTTIType* aType)
{
    return aType && IsArrayType(aType->GetName());
}

bool Red::TweakDBReflection::IsForeignKey(Red::CName aTypeName)
{
    return aTypeName == Red::ERTDBFlatType::TweakDBID;
}

bool Red::TweakDBReflection::IsForeignKey(const Red::CBaseRTTIType* aType)
{
    return aType && IsForeignKey(aType->GetName());
}

bool Red::TweakDBReflection::IsForeignKeyArray(Red::CName aTypeName)
{
    return aTypeName == Red::ERTDBFlatType::TweakDBIDArray;
}

bool Red::TweakDBReflection::IsForeignKeyArray(const Red::CBaseRTTIType* aType)
{
    return aType && IsForeignKeyArray(aType->GetName());
}

bool Red::TweakDBReflection::IsResRefToken(Red::CName aTypeName)
{
    return aTypeName == ScriptResRefTypeName || aTypeName == ResRefTypeName;
}

bool Red::TweakDBReflection::IsResRefToken(const Red::CBaseRTTIType* aType)
{
    return aType && IsResRefToken(aType->GetName());
}

bool Red::TweakDBReflection::IsResRefTokenArray(Red::CName aTypeName)
{
    return aTypeName == ScriptResRefArrayTypeName || aTypeName == ResRefArrayTypeName;
}

bool Red::TweakDBReflection::IsResRefTokenArray(const Red::CBaseRTTIType* aType)
{
    return aType && IsResRefTokenArray(aType->GetName());
}

Red::CName Red::TweakDBReflection::GetArrayTypeName(Red::CName aTypeName)
{
    // clang-format off
    switch (aTypeName)
    {
    case Red::ERTDBFlatType::Int: return Red::ERTDBFlatType::IntArray;
    case Red::ERTDBFlatType::Float: return Red::ERTDBFlatType::FloatArray;
    case Red::ERTDBFlatType::Bool: return Red::ERTDBFlatType::BoolArray;
    case Red::ERTDBFlatType::String: return Red::ERTDBFlatType::StringArray;
    case Red::ERTDBFlatType::CName: return Red::ERTDBFlatType::CNameArray;
    case Red::ERTDBFlatType::LocKey: return Red::ERTDBFlatType::LocKeyArray;
    case Red::ERTDBFlatType::ResRef: return Red::ERTDBFlatType::ResRefArray;
    case Red::ERTDBFlatType::TweakDBID: return Red::ERTDBFlatType::TweakDBIDArray;
    case Red::ERTDBFlatType::Quaternion: return Red::ERTDBFlatType::QuaternionArray;
    case Red::ERTDBFlatType::EulerAngles: return Red::ERTDBFlatType::EulerAnglesArray;
    case Red::ERTDBFlatType::Vector3: return Red::ERTDBFlatType::Vector3Array;
    case Red::ERTDBFlatType::Vector2: return Red::ERTDBFlatType::Vector2Array;
    case Red::ERTDBFlatType::Color: return Red::ERTDBFlatType::ColorArray;
    default: return {};
    }
    // clang-format on
}

Red::CName Red::TweakDBReflection::GetArrayTypeName(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetArrayTypeName(aType->GetName());
}

Red::CName Red::TweakDBReflection::GetElementTypeName(Red::CName aTypeName)
{
    // clang-format off
    switch (aTypeName)
    {
    case Red::ERTDBFlatType::IntArray: return Red::ERTDBFlatType::Int;
    case Red::ERTDBFlatType::FloatArray: return Red::ERTDBFlatType::Float;
    case Red::ERTDBFlatType::BoolArray: return Red::ERTDBFlatType::Bool;
    case Red::ERTDBFlatType::StringArray: return Red::ERTDBFlatType::String;
    case Red::ERTDBFlatType::CNameArray: return Red::ERTDBFlatType::CName;
    case Red::ERTDBFlatType::TweakDBIDArray: return Red::ERTDBFlatType::TweakDBID;
    case Red::ERTDBFlatType::LocKeyArray: return Red::ERTDBFlatType::LocKey;
    case Red::ERTDBFlatType::ResRefArray: return Red::ERTDBFlatType::ResRef;
    case Red::ERTDBFlatType::QuaternionArray: return Red::ERTDBFlatType::Quaternion;
    case Red::ERTDBFlatType::EulerAnglesArray: return Red::ERTDBFlatType::EulerAngles;
    case Red::ERTDBFlatType::Vector3Array: return Red::ERTDBFlatType::Vector3;
    case Red::ERTDBFlatType::Vector2Array: return Red::ERTDBFlatType::Vector2;
    case Red::ERTDBFlatType::ColorArray: return Red::ERTDBFlatType::Color;
    default: return {};
    }
    // clang-format on
}

Red::CName Red::TweakDBReflection::GetElementTypeName(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetElementTypeName(aType->GetName());
}

Red::InstancePtr<> Red::TweakDBReflection::Construct(Red::CName aTypeName)
{
    // clang-format off
    switch (aTypeName)
    {
    case Red::ERTDBFlatType::Int: return Red::MakeInstance<int>();
    case Red::ERTDBFlatType::Float: return Red::MakeInstance<float>();
    case Red::ERTDBFlatType::Bool: return Red::MakeInstance<bool>();
    case Red::ERTDBFlatType::String: return Red::MakeInstance<Red::CString>();
    case Red::ERTDBFlatType::CName: return Red::MakeInstance<Red::CName>();
    case Red::ERTDBFlatType::LocKey: return Red::MakeInstance<Red::LocKeyWrapper>();
    case Red::ERTDBFlatType::ResRef: return Red::MakeInstance<Red::ResourceAsyncReference<>>();
    case Red::ERTDBFlatType::TweakDBID: return Red::MakeInstance<Red::TweakDBID>();
    case Red::ERTDBFlatType::Quaternion: return Red::MakeInstance<Red::Quaternion>();
    case Red::ERTDBFlatType::EulerAngles: return Red::MakeInstance<Red::EulerAngles>();
    case Red::ERTDBFlatType::Vector3: return Red::MakeInstance<Red::Vector3>();
    case Red::ERTDBFlatType::Vector2: return Red::MakeInstance<Red::Vector2>();
    case Red::ERTDBFlatType::Color: return Red::MakeInstance<Red::Color>();
    case Red::ERTDBFlatType::IntArray: return Red::MakeInstance<Red::DynArray<int>>();
    case Red::ERTDBFlatType::FloatArray: return Red::MakeInstance<Red::DynArray<float>>();
    case Red::ERTDBFlatType::BoolArray: return Red::MakeInstance<Red::DynArray<bool>>();
    case Red::ERTDBFlatType::StringArray: return Red::MakeInstance<Red::DynArray<Red::CString>>();
    case Red::ERTDBFlatType::CNameArray: return Red::MakeInstance<Red::DynArray<Red::CName>>();
    case Red::ERTDBFlatType::LocKeyArray: return Red::MakeInstance<Red::DynArray<Red::LocKeyWrapper>>();
    case Red::ERTDBFlatType::ResRefArray: return Red::MakeInstance<Red::DynArray<Red::ResourceAsyncReference<>>>();
    case Red::ERTDBFlatType::TweakDBIDArray: return Red::MakeInstance<Red::DynArray<Red::TweakDBID>>();
    case Red::ERTDBFlatType::QuaternionArray: return Red::MakeInstance<Red::DynArray<Red::Quaternion>>();
    case Red::ERTDBFlatType::EulerAnglesArray: return Red::MakeInstance<Red::DynArray<Red::EulerAngles>>();
    case Red::ERTDBFlatType::Vector3Array: return Red::MakeInstance<Red::DynArray<Red::Vector3>>();
    case Red::ERTDBFlatType::Vector2Array: return Red::MakeInstance<Red::DynArray<Red::Vector2>>();
    case Red::ERTDBFlatType::ColorArray: return Red::MakeInstance<Red::DynArray<Red::Color>>();
    default: return {};
    }
    // clang-format on
}

Red::InstancePtr<> Red::TweakDBReflection::Construct(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return Construct(aType->GetName());
}

void Red::TweakDBReflection::RegisterExtraFlat(Red::CName aRecordType, const std::string& aPropName,
                                               Red::CName aPropType, Red::CName aForeignType)
{
    s_extraFlats[aRecordType].push_back({aPropType, aForeignType, NameSeparator + aPropName});
}

void Red::TweakDBReflection::RegisterDescendants(Red::TweakDBID aParentId,
                                                 const Core::Set<Red::TweakDBID>& aDescendantIds)
{
    s_descendantMap[aParentId].insert(aDescendantIds.begin(), aDescendantIds.end());

    for (const auto& descendantId : aDescendantIds)
    {
        s_parentMap[descendantId] = aParentId;
    }
}

bool Red::TweakDBReflection::IsOriginalRecord(Red::TweakDBID aRecordId)
{
    return s_parentMap.contains(aRecordId);
}

bool Red::TweakDBReflection::IsOriginalBaseRecord(Red::TweakDBID aParentId)
{
    return s_descendantMap.contains(aParentId);
}

Red::TweakDBID Red::TweakDBReflection::GetOriginalParent(Red::TweakDBID aRecordId)
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

Red::TweakDB* Red::TweakDBReflection::GetTweakDB()
{
    return m_tweakDb;
}

Red::IRTTISystem* Red::TweakDBReflection::GetRTTI()
{
    if (!s_rtti)
        s_rtti = Red::CRTTISystem::Get();
    ;
    return s_rtti;
}

Core::SharedPtr<const Red::TweakDBRecordInfo> Red::TweakDBReflection::RegisterRecordInfo(
    const Core::SharedPtr<Red::TweakDBRecordInfo>& aRecordInfo)
{
    std::unique_lock lockRW(m_mutex);
    if (auto [it, success] = m_resolved.insert({aRecordInfo->GetName(), aRecordInfo}); success)
    {
        return it->second;
    }
    return nullptr;
}
