#include "Reflection.hpp"
#include "Alias.hpp"
#include "Red/TweakDB/Source/Grammar.hpp"
#include "Red/TweakDB/Source/Source.hpp"
#include "ScriptableRecordClosureRegistry.hpp"
#include "ScriptableTweakDBRecord.hpp"

namespace
{
constexpr auto RecordTypePrefix = "gamedata";
constexpr auto RecordTypePrefixLength = std::char_traits<char>::length(RecordTypePrefix);
constexpr auto RecordTypeSuffix = "_Record";
constexpr auto RecordTypeSuffixLength = std::char_traits<char>::length(RecordTypeSuffix);

constexpr auto BaseRecordTypeName = Red::GetTypeName<Red::TweakDBRecord>();

constexpr auto ResRefTypeName = Red::GetTypeName<Red::RaRef<Red::CResource>>();
constexpr auto ResRefArrayTypeName = Red::GetTypeName<Red::DynArray<Red::RaRef<Red::CResource>>>();

constexpr auto ScriptResRefTypeName = Red::GetTypeName<Red::ResRef>();
constexpr auto ScriptResRefArrayTypeName = Red::GetTypeName<Red::DynArray<Red::ResRef>>();

constexpr auto NameSeparator = Red::TweakGrammar::Name::Separator;
constexpr auto PropSeparator = std::string_view(NameSeparator);
} // namespace

Red::TweakDBReflection::TweakDBReflection()
    : TweakDBReflection(Red::TweakDB::Get())
{
}

Red::TweakDBReflection::TweakDBReflection(Red::TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
    , m_rtti(Red::CRTTISystem::Get())
    , m_scriptableRecordClosures(Core::MakeUnique<ScriptableRecordClosureRegistry>())
{
}

Red::TweakDBReflection::~TweakDBReflection() = default;

Red::RecordInfo Red::TweakDBReflection::GetRecordInfo(Red::CClass* aType, const bool aCollect)
{
    if (!IsRecordType(aType))
    {
        return nullptr;
    }

    {
        std::shared_lock lockR(m_mutex);
        if (const auto iter = m_recordInfoByName.find(aType->GetName()); iter != m_recordInfoByName.end())
            return iter->second;
    }

    if (aCollect)
    {
        return CollectRecordInfo(aType);
    }

    return nullptr;
}

Red::RecordInfo Red::TweakDBReflection::GetRecordInfo(const std::string& aTypeName, const bool aCollect)
{
    return GetRecordInfo(GetRecordFullName(aTypeName), aCollect);
}

Red::RecordInfo Red::TweakDBReflection::GetRecordInfo(Red::CName aTypeName, const bool aCollect)
{
    {
        std::shared_lock lockR(m_mutex);
        if (const auto iter = m_recordInfoByName.find(aTypeName); iter != m_recordInfoByName.end())
        {
            return iter->second;
        }
    }

    if (aCollect)
    {
        return CollectRecordInfo(m_rtti->GetClass(aTypeName));
    }

    return nullptr;
}

Red::RecordInfo Red::TweakDBReflection::GetRecordInfo(const Red::CClass* aType, const bool aCollect)
{
    return GetRecordInfo(const_cast<Red::CClass*>(aType), aCollect);
}

Red::RecordInfo Red::TweakDBReflection::GetRecordInfoByHash(const uint32_t aHash)
{
    std::shared_lock lockR(m_mutex);
    if (const auto iter = m_recordInfoByHash.find(aHash); iter != m_recordInfoByHash.end())
    {
        return iter->second;
    }
    return nullptr;
}

Red::RecordInfo Red::TweakDBReflection::GetScriptableRecordInfo(const uint32_t aHash)
{
    if (const auto recordInfo = GetRecordInfoByHash(aHash); recordInfo && recordInfo->isScriptable)
    {
        return recordInfo;
    }
    return nullptr;
}

Red::RecordInfo Red::TweakDBReflection::CollectRecordInfo(Red::CClass* aType, Red::TweakDBID aSampleId)
{
    if (!IsRecordType(aType))
    {
        return nullptr;
    }

    auto sampleId = aSampleId;
    if (!sampleId.IsValid())
    {
        sampleId = GetRecordSampleId(aType);

        if (!sampleId.IsValid())
            return nullptr;
    }

    const auto recordInfo = CreateRecordInfo(aType);

    if (const auto parentInfo = CollectRecordInfo(aType->parent, sampleId))
    {
        InheritRecordInfo(recordInfo, parentInfo);
    }

    for (uint32_t funcIndex = 0u; funcIndex < aType->funcs.Size(); ++funcIndex)
    {
        const auto func = aType->funcs[funcIndex];

        auto propName = ResolvePropertyName(sampleId, func->shortName);

        // Case: Foreign Key Array => TweakDBID[]
        if (!func->returnType)
        {
            const auto arrayType = reinterpret_cast<Red::CRTTIArrayType*>(func->params[0]->type);
            const auto handleType = reinterpret_cast<Red::CRTTIWeakHandleType*>(arrayType->innerType);
            const auto recordType = reinterpret_cast<Red::CClass*>(handleType->innerType);

            const auto propInfo = CreatePropertyInfo(propName, Red::ERTDBFlatType::TweakDBIDArray);
            propInfo->foreignType = recordType;
            RegisterPropertyInfo(recordInfo, propInfo);

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

                const auto propInfo = CreatePropertyInfo(propName, Red::ERTDBFlatType::TweakDBID);
                propInfo->foreignType = recordType;
                RegisterPropertyInfo(recordInfo, propInfo);

                // Skip related function:
                // func Get[Prop]Handle()
                funcIndex += 1;
                break;
            }
            case Red::ERTTIType::Array:
            {
                if (IsResRefTokenArray(returnType))
                {
                    RegisterPropertyInfo(recordInfo, CreatePropertyInfo(propName, Red::ERTDBFlatType::ResRefArray));

                    // Skip related functions:
                    // func Get[Prop]Count()
                    // func Get[Prop]Item()
                    funcIndex += 2;
                }
                else
                {
                    RegisterPropertyInfo(recordInfo, CreatePropertyInfo(propName, returnType));

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
                    RegisterPropertyInfo(recordInfo, CreatePropertyInfo(propName, Red::ERTDBFlatType::ResRef));
                }
                else if (returnType->GetType() == Red::ERTTIType::Name)
                {
                    // Getter for LocKey returns CName, so we have to get
                    // the actual property type from the flat value.
                    auto propId = sampleId + PropSeparator + propName;
                    auto flat = m_tweakDb->GetFlatValue(propId);

                    RegisterPropertyInfo(recordInfo, CreatePropertyInfo(propName, flat->GetValue().type));
                }
                else
                {
                    RegisterPropertyInfo(recordInfo, CreatePropertyInfo(propName, returnType));
                }
            }
            }
        }
    }

    {
        auto extraFlatsIt = s_extraFlats.find(aType->name);
        if (extraFlatsIt != s_extraFlats.end())
        {
            for (const auto& extraFlat : extraFlatsIt.value())
            {
                auto propInfo = CreatePropertyInfo(extraFlat.appendix.c_str() + 1, extraFlat.typeName);
                propInfo->isExtra = true;

                if (!extraFlat.foreignTypeName.IsNone())
                {
                    propInfo->foreignType = m_rtti->GetClass(extraFlat.foreignTypeName);
                    propInfo->isForeignKey = true;
                }

                RegisterPropertyInfo(recordInfo, propInfo);
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

    assert(IsValid(recordInfo));

    RegisterRecordInfo(recordInfo);

    return recordInfo;
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
    const std::string shortName = GetRecordShortName(aName.c_str());
    return Red::Murmur3_32(reinterpret_cast<const uint8_t*>(shortName.data()), shortName.size());
}

uint32_t Red::TweakDBReflection::GetRecordTypeHash(const Red::CClass* aType)
{
    std::string_view name(aType->name.ToString());
    name.remove_prefix(RecordTypePrefixLength);
    name.remove_suffix(RecordTypeSuffixLength);

    return Red::Murmur3_32(reinterpret_cast<const uint8_t*>(name.data()), name.size());
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

std::optional<int32_t> Red::TweakDBReflection::ResolveDefaultValue(const Red::CClass* aType,
                                                                   const std::string& aPropName)
{
    std::string defaultFlatName = TweakSource::SchemaPackage;
    defaultFlatName.append(NameSeparator);
    defaultFlatName.append(GetRecordShortName(aType->GetName()));

    if (!aPropName.starts_with(NameSeparator))
    {
        defaultFlatName.append(NameSeparator);
    }

    defaultFlatName.append(aPropName);

    const auto defaultFlatId = Red::TweakDBID(defaultFlatName);

    std::shared_lock<Red::SharedSpinLock> flatLockR(m_tweakDb->mutex00);

    auto defaultFlat = m_tweakDb->flats.Find(defaultFlatId);

    if (defaultFlat == m_tweakDb->flats.End())
        return std::nullopt;

    return defaultFlat->ToTDBOffset();
}

const Red::CBaseRTTIType* Red::TweakDBReflection::GetFlatType(uint64_t aType)
{
    // clang-format off
    switch (aType)
    {
        case Red::ERTDBFlatType::Int: return TypeLocator<Red::ERTDBFlatType::Int>::Get();
        case Red::ERTDBFlatType::Float: return TypeLocator<Red::ERTDBFlatType::Float>::Get();
        case Red::ERTDBFlatType::Bool: return TypeLocator<Red::ERTDBFlatType::Bool>::Get();
        case Red::ERTDBFlatType::String: return TypeLocator<Red::ERTDBFlatType::String>::Get();
        case Red::ERTDBFlatType::CName: return TypeLocator<Red::ERTDBFlatType::CName>::Get();
        case Red::ERTDBFlatType::LocKey: return TypeLocator<Red::ERTDBFlatType::LocKey>::Get();
        case Red::ERTDBFlatType::ResRef: return TypeLocator<Red::ERTDBFlatType::ResRef>::Get();
        case Red::ERTDBFlatType::TweakDBID: return TypeLocator<Red::ERTDBFlatType::TweakDBID>::Get();
        case Red::ERTDBFlatType::Quaternion: return TypeLocator<Red::ERTDBFlatType::Quaternion>::Get();
        case Red::ERTDBFlatType::EulerAngles: return TypeLocator<Red::ERTDBFlatType::EulerAngles>::Get();
        case Red::ERTDBFlatType::Vector3: return TypeLocator<Red::ERTDBFlatType::Vector3>::Get();
        case Red::ERTDBFlatType::Vector2: return TypeLocator<Red::ERTDBFlatType::Vector2>::Get();
        case Red::ERTDBFlatType::Color: return TypeLocator<Red::ERTDBFlatType::Color>::Get();
        case Red::ERTDBFlatType::IntArray: return TypeLocator<Red::ERTDBFlatType::IntArray>::Get();
        case Red::ERTDBFlatType::FloatArray: return TypeLocator<Red::ERTDBFlatType::FloatArray>::Get();
        case Red::ERTDBFlatType::BoolArray: return TypeLocator<Red::ERTDBFlatType::BoolArray>::Get();
        case Red::ERTDBFlatType::StringArray: return TypeLocator<Red::ERTDBFlatType::StringArray>::Get();
        case Red::ERTDBFlatType::CNameArray: return TypeLocator<Red::ERTDBFlatType::CNameArray>::Get();
        case Red::ERTDBFlatType::LocKeyArray: return TypeLocator<Red::ERTDBFlatType::LocKeyArray>::Get();
        case Red::ERTDBFlatType::ResRefArray: return TypeLocator<Red::ERTDBFlatType::ResRefArray>::Get();
        case Red::ERTDBFlatType::TweakDBIDArray: return TypeLocator<Red::ERTDBFlatType::TweakDBIDArray>::Get();
        case Red::ERTDBFlatType::QuaternionArray: return TypeLocator<Red::ERTDBFlatType::QuaternionArray>::Get();
        case Red::ERTDBFlatType::EulerAnglesArray: return TypeLocator<Red::ERTDBFlatType::EulerAnglesArray>::Get();
        case Red::ERTDBFlatType::Vector3Array: return TypeLocator<Red::ERTDBFlatType::Vector3Array>::Get();
        case Red::ERTDBFlatType::Vector2Array: return TypeLocator<Red::ERTDBFlatType::Vector2Array>::Get();
        case Red::ERTDBFlatType::ColorArray: return TypeLocator<Red::ERTDBFlatType::ColorArray>::Get();
        default: return nullptr;
    }
    // clang-format on
}

const Red::CBaseRTTIType* Red::TweakDBReflection::GetFlatType(Red::CName aTypeName)
{
    const Red::CBaseRTTIType* type = m_rtti->GetType(aTypeName);

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
    auto aFullName = GetRecordFullName(aTypeName);

    Red::CClass* type = m_rtti->GetClass(aFullName);

    if (!IsRecordType(type))
        return nullptr;

    return type;
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetArrayType(Red::CName aTypeName)
{
    return m_rtti->GetType(GetArrayTypeName(aTypeName));
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetArrayType(const Red::CBaseRTTIType* aType)
{
    return m_rtti->GetType(GetArrayTypeName(aType));
}

Red::CBaseRTTIType* Red::TweakDBReflection::GetElementType(Red::CName aTypeName)
{
    return GetElementType(m_rtti->GetType(aTypeName));
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

Red::CName Red::TweakDBReflection::GetRecordFullName(Red::CName aName, const bool aRegister)
{
    return GetRecordFullName(aName.ToString(), aRegister);
}

Red::CName Red::TweakDBReflection::GetRecordFullName(const char* aName, const bool aRegister)
{
    std::string finalName = aName;

    if (finalName.empty())
        return {};

    if (!finalName.starts_with(RecordTypePrefix))
        finalName.insert(0, RecordTypePrefix);

    if (!finalName.ends_with(RecordTypeSuffix))
        finalName.append(RecordTypeSuffix);

    return aRegister ? CNamePool::Add(finalName.c_str()) : Red::CName(finalName.c_str());
}

Red::CName Red::TweakDBReflection::GetRecordFullName(const std::string& aName, bool aRegister)
{
    return GetRecordFullName(aName.c_str(), aRegister);
}

Red::CName Red::TweakDBReflection::GetRecordAliasName(Red::CName aName, const bool aRegister)
{
    return GetRecordAliasName(aName.ToString(), aRegister);
}

Red::CName Red::TweakDBReflection::GetRecordAliasName(const char* aName, const bool aRegister)
{
    std::string finalName = aName;

    if (finalName.empty())
        return {};

    if (finalName.starts_with(RecordTypePrefix))
        finalName.erase(0, RecordTypePrefixLength);

    if (!finalName.ends_with(RecordTypeSuffix))
        finalName.append(RecordTypeSuffix);

    return aRegister ? CNamePool::Add(finalName.c_str()) : Red::CName(finalName.c_str());
}

std::string Red::TweakDBReflection::GetRecordShortName(Red::CName aName)
{
    return GetRecordShortName(aName.ToString());
}

std::string Red::TweakDBReflection::GetRecordShortName(const char* aName)
{
    std::string finalName = aName;

    if (finalName.starts_with(RecordTypePrefix))
        finalName.erase(0, RecordTypePrefixLength);

    if (finalName.ends_with(RecordTypeSuffix))
        finalName.erase(finalName.end() - RecordTypeSuffixLength, finalName.end());

    return finalName;
}

std::string Red::TweakDBReflection::GetPropertyFunctionName(Red::CName aName)
{
    std::string propName = aName.ToString();
    propName[0] = static_cast<char>(std::toupper(propName[0]));
    return propName;
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

Red::RecordInfo Red::TweakDBReflection::RegisterScriptableRecordType(const std::string& aName, Red::CClass* aParent)
{
    const auto name = GetRecordFullName(aName.c_str(), true);

    if (const auto recordInfo = GetRecordInfo(name))
    {
        return recordInfo;
    }

    const auto recordInfo = CreateRecordInfo(name);
    recordInfo->isScriptable = true;
    recordInfo->type = CreateRecordClass(recordInfo->name, recordInfo->aliasName, recordInfo->typeHash);

    if (!recordInfo->type)
    {
        return nullptr;
    }

    // This won't work because class registration and description need to happen
    // in 2 phases so type inheritance can work correctly. In other words, if
    // class B is registered before class A, and B extends A, class B's parent
    // will not be set correctly.
    if (aParent && IsRecordType(aParent))
    {
        recordInfo->type->parent = aParent;

        if (const auto parentInfo = GetRecordInfo(aParent))
        {
            InheritRecordInfo(recordInfo, parentInfo);
        }
    }
    else
    {
        recordInfo->type->parent = ScriptableTweakDBRecord::TYPE::GetClass();
    }

    if (!RegisterRecordInfo(recordInfo))
    {
        m_rtti->UnregisterType(recordInfo->type);
        return nullptr;
    }

    return recordInfo;
}

Red::PropertyInfo Red::TweakDBReflection::RegisterScriptableProperty(const RecordInfo& aRecord,
                                                                     const std::string& aPropertyName,
                                                                     const uint64_t aFlatType,
                                                                     const Red::CClass* aForeignType)
{
    if (!aRecord)
    {
        return nullptr;
    }

    const auto propertyInfo = CreatePropertyInfo(aPropertyName, aFlatType);

    // Like parent in the registration of scriptable records, foreignType will not work
    // here due to race conditions. We need to split up registration and description
    // phases so that all types are registered before we try to resolve any foreign keys.
    propertyInfo->foreignType = aForeignType;

    const auto closure = m_scriptableRecordClosures->CreateClosure(this, propertyInfo);

    if (!closure)
    {
        return nullptr;
    }

    auto* function = Red::CClassFunction::Create(aRecord->type, propertyInfo->functionName.ToString(),
                                                 propertyInfo->functionName.ToString(), closure);
    function->SetReturnType(propertyInfo->type->GetName());
    aRecord->type->RegisterFunction(function);

    if (!RegisterPropertyInfo(aRecord, propertyInfo))
    {
        m_scriptableRecordClosures->DestroyClosure(closure);
        return nullptr;
    }

    return propertyInfo;
}

Red::Handle<Red::ScriptableTweakDBRecord> Red::TweakDBReflection::ConstructScriptableRecord(Red::TweakDBID aRecordId,
                                                                                            uint32_t aHash)
{
    if (const auto recordInfo = GetScriptableRecordInfo(aHash))
    {
        if (const auto* cls = recordInfo->type)
        {
            const auto instance = Red::MakeScriptedHandle<Red::ScriptableTweakDBRecord>(const_cast<Red::CClass*>(cls));
            instance->recordID = aRecordId;
            return instance;
        }
    }
    return {};
}

void* Red::TweakDBReflection::GetScriptablePropertyValue(const Red::ScriptableTweakDBRecord* aRecord,
                                                         const PropertyInfo& aPropertyInfo)
{
    if (auto* flat = m_tweakDb->GetFlatValue(aRecord->recordID + aPropertyInfo->appendix); flat)
    {
        if (const auto value = flat->GetValue(); value.value)
            return value.value;
    }

    return nullptr;
}

bool Red::TweakDBReflection::RegisterRecordInfo(RecordInfo aRecordInfo)
{
    if (!IsValid(aRecordInfo))
        return false;

    std::unique_lock lockRW(m_mutex);
    if (const auto& [_, success] = m_recordInfoByName.insert({aRecordInfo->name, aRecordInfo}); success)
    {
        m_recordInfoByHash[aRecordInfo->typeHash] = aRecordInfo;
        return true;
    }

    return false;
}

void Red::TweakDBReflection::InheritRecordInfo(const RecordInfo& aRecordInfo, Red::RecordInfo aParentInfo)
{
    if (!aParentInfo)
        return;

    aRecordInfo->parent = aParentInfo->type;

    for (const auto parentProp : aParentInfo->props | std::views::values)
        RegisterPropertyInfo(aRecordInfo, parentProp);
}

bool Red::TweakDBReflection::RegisterPropertyInfo(const RecordInfo& aRecordInfo, const PropertyInfo& aPropertyInfo)
{
    if (!IsValid(aPropertyInfo))
        return false;

    aRecordInfo->props[aPropertyInfo->name] = aPropertyInfo;

    return true;
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

bool Red::TweakDBReflection::IsValid(const PropertyInfo& aPropInfo)
{
    if (aPropInfo->name.IsNone() || !aPropInfo->type || aPropInfo->appendix.length() < 2 ||
        !IsFlatType(aPropInfo->type->GetName()) || !aPropInfo->appendix.starts_with(NameSeparator))
    {
        return false;
    }

    if (!aPropInfo->isExtra && aPropInfo->functionName.IsNone())
    {
        return false;
    }

    switch (aPropInfo->type->GetName())
    {
    case ERTDBFlatType::Int:
    case ERTDBFlatType::Float:
    case ERTDBFlatType::Bool:
    case ERTDBFlatType::String:
    case ERTDBFlatType::CName:
    case ERTDBFlatType::LocKey:
    case ERTDBFlatType::ResRef:
    case ERTDBFlatType::Quaternion:
    case ERTDBFlatType::EulerAngles:
    case ERTDBFlatType::Vector3:
    case ERTDBFlatType::Vector2:
    case ERTDBFlatType::Color:
        return !aPropInfo->isArray && !aPropInfo->elementType && !aPropInfo->isForeignKey && !aPropInfo->foreignType;
    case ERTDBFlatType::IntArray:
    case ERTDBFlatType::FloatArray:
    case ERTDBFlatType::BoolArray:
    case ERTDBFlatType::StringArray:
    case ERTDBFlatType::CNameArray:
    case ERTDBFlatType::LocKeyArray:
    case ERTDBFlatType::ResRefArray:
    case ERTDBFlatType::QuaternionArray:
    case ERTDBFlatType::EulerAnglesArray:
    case ERTDBFlatType::Vector3Array:
    case ERTDBFlatType::Vector2Array:
    case ERTDBFlatType::ColorArray:
        return aPropInfo->isArray && aPropInfo->elementType && !aPropInfo->isForeignKey && !aPropInfo->foreignType &&
               aPropInfo->elementType->GetName() == GetElementTypeName(aPropInfo->type);
    case ERTDBFlatType::TweakDBID:
        return !aPropInfo->isArray && !aPropInfo->elementType && aPropInfo->isForeignKey && aPropInfo->foreignType;
    case ERTDBFlatType::TweakDBIDArray:
        return aPropInfo->isArray && aPropInfo->elementType && aPropInfo->isForeignKey && aPropInfo->foreignType &&
               aPropInfo->elementType->GetName() == ERTDBFlatType::TweakDBID;
    default:
        return false;
    }
}

bool Red::TweakDBReflection::IsValid(const RecordInfo& aRecordInfo)
{
    if (aRecordInfo->name.IsNone() || aRecordInfo->aliasName.IsNone() || aRecordInfo->shortName.empty() ||
        !aRecordInfo->type || aRecordInfo->type->GetName() != aRecordInfo->name)
    {
        return false;
    }

    if (aRecordInfo->parent && !IsRecordType(aRecordInfo->parent))
    {
        return false;
    }

    return true;
}

Red::TweakDBID Red::TweakDBReflection::BuildRTDBID(const std::string& aRecordName, Red::CName aPropertyName)
{
    std::string id = TweakSource::SchemaPackage;
    id.append(NameSeparator);
    id.append(aRecordName);
    id.append(NameSeparator);
    id.append(aPropertyName.ToString());
    return TweakDBID{id};
}

Red::RecordInfo Red::TweakDBReflection::CreateRecordInfo(Red::CName aName)
{
    const auto recordInfo = Core::MakeShared<Red::TweakDBRecordInfo>();
    recordInfo->name = aName;
    recordInfo->aliasName = GetRecordAliasName(recordInfo->name, true);
    recordInfo->shortName = GetRecordShortName(recordInfo->name);
    recordInfo->typeHash = GetRecordTypeHash(recordInfo->shortName);
    return recordInfo;
}

Red::RecordInfo Red::TweakDBReflection::CreateRecordInfo(Red::CClass* aClass)
{
    const auto recordInfo = Core::MakeShared<Red::TweakDBRecordInfo>();
    recordInfo->name = aClass->GetName();
    recordInfo->aliasName = GetRecordAliasName(recordInfo->name);
    recordInfo->shortName = GetRecordShortName(recordInfo->name);
    recordInfo->type = aClass;
    recordInfo->typeHash = GetRecordTypeHash(recordInfo->type);
    return recordInfo;
}

Red::PropertyInfo Red::TweakDBReflection::CreatePropertyInfo(const std::string& aName, const uint64_t aFlatType)
{
    return CreatePropertyInfo(aName, GetFlatType(aFlatType));
}

Red::PropertyInfo Red::TweakDBReflection::CreatePropertyInfo(const std::string& aName,
                                                             const Red::CBaseRTTIType* aFlatType)
{
    const auto propInfo = Core::MakeShared<Red::TweakDBPropertyInfo>();
    propInfo->name = CNamePool::Add(aName.c_str());
    propInfo->type = aFlatType;
    propInfo->appendix = PropSeparator;
    propInfo->appendix.append(propInfo->name.ToString());
    propInfo->functionName = CNamePool::Add(GetPropertyFunctionName(propInfo->name).c_str());

    if (IsArrayType(propInfo->type))
    {
        propInfo->isArray = true;
        propInfo->elementType = GetElementType(propInfo->type);
    }

    if (IsForeignKey(propInfo->type) || IsForeignKeyArray(propInfo->type))
    {
        propInfo->isForeignKey = true;
    }

    return propInfo;
}

Red::RecordClass* Red::TweakDBReflection::GetRecordClass(Red::CName aName)
{
    std::shared_lock lockR(m_recordClassMutex);
    if (const auto it = m_scriptableRecordClasses.find(aName); it != m_scriptableRecordClasses.end())
    {
        return it->second.get();
    }
    return nullptr;
}

Red::RecordClass* Red::TweakDBReflection::CreateRecordClass(Red::CName aName, Red::CName aAliasName, uint32_t aHash)
{
    if (auto* cls = GetRecordClass(aName))
    {
        return cls;
    }

    if (m_rtti->GetClass(aName))
    {
        return nullptr;
    }

    const auto cls = Core::MakeShared<RecordClass>(aName, aHash);
    m_rtti->RegisterType(cls.get());
    m_rtti->RegisterScriptName(aName, aAliasName);

    {
        std::unique_lock lockRW(m_recordClassMutex);
        m_scriptableRecordClasses[aName] = cls;
    }

    return cls.get();
}
