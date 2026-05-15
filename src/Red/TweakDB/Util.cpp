#include "Alias.hpp"
#include "Source/Grammar.hpp"
#include "Source/Source.hpp"

namespace
{
constexpr auto RecordTypePrefix = "gamedata";
constexpr auto RecordTypePrefixLength = std::char_traits<char>::length(RecordTypePrefix);
constexpr auto RecordTypeSuffix = "_Record";
constexpr auto RecordTypeSuffixLength = std::char_traits<char>::length(RecordTypeSuffix);

constexpr auto ResRefTypeName = Red::GetTypeName<Red::RaRef<Red::CResource>>();
constexpr auto ResRefArrayTypeName = Red::GetTypeName<Red::DynArray<Red::RaRef<Red::CResource>>>();

constexpr auto ScriptResRefTypeName = Red::GetTypeName<Red::ResRef>();
constexpr auto ScriptResRefArrayTypeName = Red::GetTypeName<Red::DynArray<Red::ResRef>>();

constexpr auto BaseRecordTypeName = Red::GetTypeName<Red::TweakDBRecord>();

constexpr auto NameSeparator = Red::TweakGrammar::Name::Separator;
constexpr auto PropSeparator = std::string_view(NameSeparator);
} // namespace

namespace Red
{
CBaseRTTIType* GetFlatType(const uint64_t aType)
{
    // clang-format off
    switch (aType)
    {
    case ERTDBFlatType::Int: return TypeLocator<ERTDBFlatType::Int>::Get();
    case ERTDBFlatType::Float: return TypeLocator<ERTDBFlatType::Float>::Get();
    case ERTDBFlatType::Bool: return TypeLocator<ERTDBFlatType::Bool>::Get();
    case ERTDBFlatType::String: return TypeLocator<ERTDBFlatType::String>::Get();
    case ERTDBFlatType::CName: return TypeLocator<ERTDBFlatType::CName>::Get();
    case ERTDBFlatType::LocKey: return TypeLocator<ERTDBFlatType::LocKey>::Get();
    case ERTDBFlatType::ResRef: return TypeLocator<ERTDBFlatType::ResRef>::Get();
    case ERTDBFlatType::TweakDBID: return TypeLocator<ERTDBFlatType::TweakDBID>::Get();
    case ERTDBFlatType::Quaternion: return TypeLocator<ERTDBFlatType::Quaternion>::Get();
    case ERTDBFlatType::EulerAngles: return TypeLocator<ERTDBFlatType::EulerAngles>::Get();
    case ERTDBFlatType::Vector3: return TypeLocator<ERTDBFlatType::Vector3>::Get();
    case ERTDBFlatType::Vector2: return TypeLocator<ERTDBFlatType::Vector2>::Get();
    case ERTDBFlatType::Color: return TypeLocator<ERTDBFlatType::Color>::Get();
    case ERTDBFlatType::IntArray: return TypeLocator<ERTDBFlatType::IntArray>::Get();
    case ERTDBFlatType::FloatArray: return TypeLocator<ERTDBFlatType::FloatArray>::Get();
    case ERTDBFlatType::BoolArray: return TypeLocator<ERTDBFlatType::BoolArray>::Get();
    case ERTDBFlatType::StringArray: return TypeLocator<ERTDBFlatType::StringArray>::Get();
    case ERTDBFlatType::CNameArray: return TypeLocator<ERTDBFlatType::CNameArray>::Get();
    case ERTDBFlatType::LocKeyArray: return TypeLocator<ERTDBFlatType::LocKeyArray>::Get();
    case ERTDBFlatType::ResRefArray: return TypeLocator<ERTDBFlatType::ResRefArray>::Get();
    case ERTDBFlatType::TweakDBIDArray: return TypeLocator<ERTDBFlatType::TweakDBIDArray>::Get();
    case ERTDBFlatType::QuaternionArray: return TypeLocator<ERTDBFlatType::QuaternionArray>::Get();
    case ERTDBFlatType::EulerAnglesArray: return TypeLocator<ERTDBFlatType::EulerAnglesArray>::Get();
    case ERTDBFlatType::Vector3Array: return TypeLocator<ERTDBFlatType::Vector3Array>::Get();
    case ERTDBFlatType::Vector2Array: return TypeLocator<ERTDBFlatType::Vector2Array>::Get();
    case ERTDBFlatType::ColorArray: return TypeLocator<ERTDBFlatType::ColorArray>::Get();
    default: return nullptr;
    }
    // clang-format on
}

CBaseRTTIType* GetFlatType(CName aTypeName)
{
    return GetFlatType(aTypeName.hash);
}

CBaseRTTIType* GetArrayType(CName aTypeName)
{
    return CRTTISystem::Get()->GetType(GetArrayTypeName(aTypeName));
}

CBaseRTTIType* GetArrayType(const CBaseRTTIType* aType)
{
    return CRTTISystem::Get()->GetType(GetArrayTypeName(aType));
}

CBaseRTTIType* GetElementType(CName aTypeName)
{
    return GetElementType(CRTTISystem::Get()->GetType(aTypeName));
}

CBaseRTTIType* GetElementType(const CBaseRTTIType* aType)
{
    if (!aType || aType->GetType() != ERTTIType::Array)
        return nullptr;

    return reinterpret_cast<const CRTTIBaseArrayType*>(aType)->innerType;
}

bool IsFlatType(CName aTypeName)
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

bool IsFlatType(const CBaseRTTIType* aType)
{
    return aType && IsFlatType(aType->GetName());
}

bool IsArrayType(CName aTypeName)
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

bool IsArrayType(const CBaseRTTIType* aType)
{
    return aType && IsArrayType(aType->GetName());
}

bool IsForeignKey(CName aTypeName)
{
    return aTypeName == ERTDBFlatType::TweakDBID;
}

bool IsForeignKey(const CBaseRTTIType* aType)
{
    return aType && IsForeignKey(aType->GetName());
}

bool IsForeignKeyArray(CName aTypeName)
{
    return aTypeName == ERTDBFlatType::TweakDBIDArray;
}

bool IsForeignKeyArray(const CBaseRTTIType* aType)
{
    return aType && IsForeignKeyArray(aType->GetName());
}

bool IsResRefToken(CName aTypeName)
{
    return aTypeName == ScriptResRefTypeName || aTypeName == ResRefTypeName;
}

bool IsResRefToken(const CBaseRTTIType* aType)
{
    return aType && IsResRefToken(aType->GetName());
}

bool IsResRefTokenArray(CName aTypeName)
{
    return aTypeName == ScriptResRefArrayTypeName || aTypeName == ResRefArrayTypeName;
}

bool IsResRefTokenArray(const CBaseRTTIType* aType)
{
    return aType && IsResRefTokenArray(aType->GetName());
}

bool IsLocKey(CName aTypeName)
{
    return aTypeName == ERTDBFlatType::LocKey;
}

bool IsLocKey(const CBaseRTTIType* aType)
{
    return aType && IsLocKey(aType->GetName());
}

bool IsLocKeyArray(CName aTypeName)
{
    return aTypeName == ERTDBFlatType::LocKeyArray;
}

bool IsLocKeyArray(const CBaseRTTIType* aType)
{
    return aType && IsLocKeyArray(aType->GetName());
}

CName GetArrayTypeName(CName aTypeName)
{
    // clang-format off
    switch (aTypeName)
    {
    case ERTDBFlatType::Int: return ERTDBFlatType::IntArray;
    case ERTDBFlatType::Float: return ERTDBFlatType::FloatArray;
    case ERTDBFlatType::Bool: return ERTDBFlatType::BoolArray;
    case ERTDBFlatType::String: return ERTDBFlatType::StringArray;
    case ERTDBFlatType::CName: return ERTDBFlatType::CNameArray;
    case ERTDBFlatType::LocKey: return ERTDBFlatType::LocKeyArray;
    case ERTDBFlatType::ResRef: return ERTDBFlatType::ResRefArray;
    case ERTDBFlatType::TweakDBID: return ERTDBFlatType::TweakDBIDArray;
    case ERTDBFlatType::Quaternion: return ERTDBFlatType::QuaternionArray;
    case ERTDBFlatType::EulerAngles: return ERTDBFlatType::EulerAnglesArray;
    case ERTDBFlatType::Vector3: return ERTDBFlatType::Vector3Array;
    case ERTDBFlatType::Vector2: return ERTDBFlatType::Vector2Array;
    case ERTDBFlatType::Color: return ERTDBFlatType::ColorArray;
    default: return {};
    }
    // clang-format on
}

CName GetArrayTypeName(const CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetArrayTypeName(aType->GetName());
}

CName GetElementTypeName(CName aTypeName)
{
    // clang-format off
    switch (aTypeName)
    {
    case ERTDBFlatType::IntArray: return ERTDBFlatType::Int;
    case ERTDBFlatType::FloatArray: return ERTDBFlatType::Float;
    case ERTDBFlatType::BoolArray: return ERTDBFlatType::Bool;
    case ERTDBFlatType::StringArray: return ERTDBFlatType::String;
    case ERTDBFlatType::CNameArray: return ERTDBFlatType::CName;
    case ERTDBFlatType::TweakDBIDArray: return ERTDBFlatType::TweakDBID;
    case ERTDBFlatType::LocKeyArray: return ERTDBFlatType::LocKey;
    case ERTDBFlatType::ResRefArray: return ERTDBFlatType::ResRef;
    case ERTDBFlatType::QuaternionArray: return ERTDBFlatType::Quaternion;
    case ERTDBFlatType::EulerAnglesArray: return ERTDBFlatType::EulerAngles;
    case ERTDBFlatType::Vector3Array: return ERTDBFlatType::Vector3;
    case ERTDBFlatType::Vector2Array: return ERTDBFlatType::Vector2;
    case ERTDBFlatType::ColorArray: return ERTDBFlatType::Color;
    default: return {};
    }
    // clang-format on
}

CName GetElementTypeName(const CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetElementTypeName(aType->GetName());
}

InstancePtr<> ConstructFlatInstance(CName aTypeName)
{
    // clang-format off
    switch (aTypeName)
    {
    case ERTDBFlatType::Int: return MakeInstance<int>();
    case ERTDBFlatType::Float: return MakeInstance<float>();
    case ERTDBFlatType::Bool: return MakeInstance<bool>();
    case ERTDBFlatType::String: return MakeInstance<CString>();
    case ERTDBFlatType::CName: return MakeInstance<CName>();
    case ERTDBFlatType::LocKey: return MakeInstance<LocKeyWrapper>();
    case ERTDBFlatType::ResRef: return MakeInstance<ResourceAsyncReference<>>();
    case ERTDBFlatType::TweakDBID: return MakeInstance<TweakDBID>();
    case ERTDBFlatType::Quaternion: return MakeInstance<Quaternion>();
    case ERTDBFlatType::EulerAngles: return MakeInstance<EulerAngles>();
    case ERTDBFlatType::Vector3: return MakeInstance<Vector3>();
    case ERTDBFlatType::Vector2: return MakeInstance<Vector2>();
    case ERTDBFlatType::Color: return MakeInstance<Color>();
    case ERTDBFlatType::IntArray: return MakeInstance<DynArray<int>>();
    case ERTDBFlatType::FloatArray: return MakeInstance<DynArray<float>>();
    case ERTDBFlatType::BoolArray: return MakeInstance<DynArray<bool>>();
    case ERTDBFlatType::StringArray: return MakeInstance<DynArray<CString>>();
    case ERTDBFlatType::CNameArray: return MakeInstance<DynArray<CName>>();
    case ERTDBFlatType::LocKeyArray: return MakeInstance<DynArray<LocKeyWrapper>>();
    case ERTDBFlatType::ResRefArray: return MakeInstance<DynArray<ResourceAsyncReference<>>>();
    case ERTDBFlatType::TweakDBIDArray: return MakeInstance<DynArray<TweakDBID>>();
    case ERTDBFlatType::QuaternionArray: return MakeInstance<DynArray<Quaternion>>();
    case ERTDBFlatType::EulerAnglesArray: return MakeInstance<DynArray<EulerAngles>>();
    case ERTDBFlatType::Vector3Array: return MakeInstance<DynArray<Vector3>>();
    case ERTDBFlatType::Vector2Array: return MakeInstance<DynArray<Vector2>>();
    case ERTDBFlatType::ColorArray: return MakeInstance<DynArray<Color>>();
    default: return {};
    }
    // clang-format on
}

InstancePtr<> ConstructFlatInstance(const CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return ConstructFlatInstance(aType->GetName());
}

ValuePtr<> ConstructFlatValue(const CBaseRTTIType* aType)
{
    if (!aType || !IsFlatType(aType))
        return {};

    return MakeValue(aType);
}

ValuePtr<> ConstructFlatValue(CName aTypeName)
{
    return ConstructFlatValue(CRTTISystem::Get()->GetType(aTypeName));
}

CClass* GetRecordType(CName aTypeName)
{
    return GetRecordType(aTypeName.ToString());
}

CClass* GetRecordType(const char* aTypeName)
{
    const auto aFullName = GetRecordFullName<CName>(aTypeName);

    CClass* type = CRTTISystem::Get()->GetClass(aFullName);

    if (!IsRecordType(type))
        return nullptr;

    return type;
}

bool IsRecordType(CName aTypeName)
{
    return aTypeName && IsRecordType(CRTTISystem::Get()->GetClass(aTypeName));
}

bool IsRecordType(const CClass* aType)
{
    static CBaseRTTIType* s_baseRecordType = CRTTISystem::Get()->GetClass(BaseRecordTypeName);

    return aType && aType != s_baseRecordType && aType->IsA(s_baseRecordType);
}

std::string NormalizeRecordName(const std::string& aName)
{
    return GetRecordFullName<std::string>(Capitalize(GetRecordShortName<std::string>(aName)));
}

template<>
std::string GetRecordFullName(const std::string& aName)
{
    std::string finalName = aName;

    if (finalName.empty())
        return {};

    if (!finalName.starts_with(RecordTypePrefix))
        finalName.insert(0, RecordTypePrefix);

    if (!finalName.ends_with(RecordTypeSuffix))
        finalName.append(RecordTypeSuffix);

    return finalName;
}

template<>
std::string GetRecordFullName(const char* aName)
{
    if (aName)
    {
        return GetRecordFullName<std::string>(std::string(aName));
    }
    return {};
}

template<>
std::string GetRecordFullName(const CName aName)
{
    return GetRecordFullName<std::string>(aName.ToString());
}

template<>
CName GetRecordFullName(const std::string& aName)
{
    return {GetRecordFullName<std::string>(aName).c_str()};
}

template<>
CName GetRecordFullName(const char* aName)
{
    if (aName)
    {
        return GetRecordFullName<CName>(std::string(aName));
    }
    return {};
}

template<>
CName GetRecordFullName(const CName aName)
{
    return GetRecordFullName<CName>(aName.ToString());
}

template<>
std::string GetRecordAliasName(const std::string& aName)
{
    std::string finalName = aName;

    if (finalName.empty())
        return {};

    if (finalName.starts_with(RecordTypePrefix))
        finalName.erase(0, RecordTypePrefixLength);

    if (!finalName.ends_with(RecordTypeSuffix))
        finalName.append(RecordTypeSuffix);

    return finalName;
}

template<>
std::string GetRecordAliasName(const char* aName)
{
    if (aName)
    {
        return GetRecordAliasName<std::string>(std::string(aName));
    }
    return {};
}

template<>
std::string GetRecordAliasName(const CName aName)
{
    return GetRecordAliasName<std::string>(aName.ToString());
}

template<>
CName GetRecordAliasName(const std::string& aName)
{
    return {GetRecordAliasName<std::string>(aName).c_str()};
}

template<>
CName GetRecordAliasName(const char* aName)
{
    if (aName)
    {
        return GetRecordAliasName<CName>(std::string(aName));
    }
    return {};
}

template<>
CName GetRecordAliasName(const CName aName)
{
    return GetRecordAliasName<CName>(aName.ToString());
}

template<>
std::string GetRecordShortName(const std::string& aName)
{
    std::string finalName = aName;

    if (finalName.starts_with(RecordTypePrefix))
        finalName.erase(0, RecordTypePrefixLength);

    if (finalName.ends_with(RecordTypeSuffix))
        finalName.erase(finalName.end() - RecordTypeSuffixLength, finalName.end());

    return finalName;
}

template<>
std::string GetRecordShortName(CName aName)
{
    return GetRecordShortName<std::string>(aName.ToString());
}

template<>
std::string GetRecordShortName(const char* aName)
{
    if (aName)
    {
        return GetRecordShortName<std::string>(std::string(aName));
    }
    return {};
}

template<>
CName GetRecordShortName(const std::string& aName)
{
    return CName{GetRecordShortName<std::string>(aName).c_str()};
}

template<>
CName GetRecordShortName(CName aName)
{
    return GetRecordShortName<CName>(aName.ToString());
}

template<>
CName GetRecordShortName(const char* aName)
{
    if (aName)
    {
        return GetRecordShortName<CName>(std::string(aName));
    }
    return {};
}

uint32_t GetRecordTypeHash(CName aName)
{
    return GetRecordTypeHash(GetRecordShortName<std::string>(aName.ToString()));
}

uint32_t GetRecordTypeHash(const std::string& aName)
{
    return GetRecordTypeHash(aName.c_str());
}

uint32_t GetRecordTypeHash(const char* aName)
{
    return Murmur3_32(reinterpret_cast<const uint8_t*>(aName), strlen(aName));
}

uint32_t GetRecordTypeHash(const CClass* aType)
{
    std::string_view name(aType->name.ToString());
    name.remove_prefix(RecordTypePrefixLength);
    name.remove_suffix(RecordTypeSuffixLength);

    return Murmur3_32(reinterpret_cast<const uint8_t*>(name.data()), name.size());
}

TweakDBID GetRTDBFlatID(CName aRecord, const std::string& aProp)
{
    return GetRTDBFlatID(aRecord, aProp.c_str());
}

TweakDBID GetRTDBFlatID(CName aRecord, const char* aProp)
{
    if (!aProp)
    {
        return {};
    }

    const auto recordID = GetRTDBRecordID(aRecord);

    if (!recordID.IsValid())
    {
        return {};
    }

    return recordID + PropSeparator + std::string_view(aProp);
}

TweakDBID GetRTDBRecordID(const std::string& aRecord)
{
    const auto name = GetRecordShortName<std::string>(aRecord);

    if (name.empty())
    {
        return {};
    }

    std::string flatName = TweakSource::SchemaPackage;
    flatName.append(PropSeparator);
    flatName.append(name);

    return {flatName.c_str()};
}

TweakDBID GetRTDBRecordID(CName aRecord)
{
    return GetRTDBRecordID(std::string(aRecord.ToString()));
}

std::string Capitalize(const std::string& aName)
{
    return Capitalize(aName.c_str());
}

std::string Capitalize(const char* aName)
{
    if (!aName)
    {
        return {};
    }

    std::string name = aName;
    if (!name.empty())
    {
        name[0] = static_cast<char>(std::toupper(name[0]));
    }
    return name;
}

std::string Decapitalize(const std::string& aName)
{
    return Decapitalize(aName.c_str());
}

std::string Decapitalize(const char* aName)
{
    if (!aName)
    {
        return {};
    }

    std::string name = aName;

    if (!name.empty())
    {
        name[0] = static_cast<char>(std::tolower(name[0]));
    }
    return name;
}

template<>
std::string GetHandleTypeName(const CClass* aClass)
{
    static constexpr auto HandlePrefix = Red::GetTypePrefixStr<Handle>();

    if (!aClass)
        return {};

    std::string name = HandlePrefix.data();
    name.append(aClass->GetName().ToString());

    return name;
}

template<>
std::string GetWHandleTypeName(const CClass* aClass)
{
    static constexpr auto WeakHandlePrefix = Red::GetTypePrefixStr<WeakHandle>();

    if (!aClass)
        return {};

    std::string name = WeakHandlePrefix.data();
    name.append(aClass->GetName().ToString());

    return name;
}

template<>
std::string GetWHandleArrayTypeName(const CClass* aClass)
{
    static constexpr auto WeakHandlePrefix = Red::GetTypePrefixStr<WeakHandle>();
    static constexpr auto ArrayPrefix = Red::GetTypePrefixStr<DynArray>();

    if (!aClass)
        return {};

    std::string name = ArrayPrefix.data();
    name.append(WeakHandlePrefix.data());
    name.append(aClass->GetName().ToString());

    return name;
}

template<>
CName GetHandleTypeName(const CClass* aClass)
{
    return {GetHandleTypeName<std::string>(aClass).c_str()};
}

template<>
CName GetWHandleTypeName(const CClass* aClass)
{
    return {GetWHandleTypeName<std::string>(aClass).c_str()};
}

template<>
CName GetWHandleArrayTypeName(const CClass* aClass)
{
    return {GetWHandleArrayTypeName<std::string>(aClass).c_str()};
}

CHandle* GetHandleType(const CClass* aClass)
{
    static auto* rtti = CRTTISystem::Get();

    if (!aClass)
        return nullptr;

    auto* type = rtti->GetType(GetHandleTypeName<CName>(aClass));

    if (!type || type->GetType() != ERTTIType::Handle)
        return nullptr;

    return reinterpret_cast<CHandle*>(type);
}

CWeakHandle* GetWHandleType(const CClass* aClass)
{
    static auto* rtti = CRTTISystem::Get();

    if (!aClass)
        return nullptr;

    auto* type = rtti->GetType(GetWHandleTypeName<CName>(aClass));

    if (!type || type->GetType() != ERTTIType::WeakHandle)
        return nullptr;

    return reinterpret_cast<CWeakHandle*>(type);
}

CRTTIBaseArrayType* GetWHandleArrayType(const CClass* aClass)
{
    static auto* rtti = CRTTISystem::Get();

    if (!aClass)
        return nullptr;

    auto* type = rtti->GetType(GetWHandleArrayTypeName<CName>(aClass));

    if (!type || type->GetType() != ERTTIType::Array)
        return nullptr;

    return reinterpret_cast<CRTTIBaseArrayType*>(type);
}

const CRTTIBaseArrayType* ToArrayType(const CBaseRTTIType* aType)
{
    if (aType->GetType() == ERTTIType::Array)
    {
        return reinterpret_cast<const CRTTIBaseArrayType*>(aType);
    }

    return nullptr;
}

const CWeakHandle* ToWeakHandleType(const CBaseRTTIType* aType)
{
    if (aType->GetType() == ERTTIType::WeakHandle)
    {
        return reinterpret_cast<const CWeakHandle*>(aType);
    }
    return nullptr;
}

const CHandle* ToHandleType(const CBaseRTTIType* aType)
{
    if (aType->GetType() == ERTTIType::Handle)
    {
        return reinterpret_cast<const CHandle*>(aType);
    }
    return nullptr;
}

template<>
const CBaseRTTIType* GetInnerType(const CBaseRTTIType* aType)
{
    if (const auto arrayType = ToArrayType(aType))
        return arrayType->GetInnerType();

    if (const auto handleType = ToWeakHandleType(aType))
        return handleType->GetInnerType();

    if (const auto handleType = ToHandleType(aType))
        return handleType->GetInnerType();

    return nullptr;
}

template<>
const CClass* GetInnerType(const CBaseRTTIType* aType)
{
    if (const auto* type = GetInnerType(aType))
    {
        if (type->GetType() == ERTTIType::Class)
            return reinterpret_cast<const CClass*>(type);

        if (const auto* innerType = GetInnerType(type); innerType && innerType->GetType() == ERTTIType::Class)
            return reinterpret_cast<const CClass*>(innerType);
    }

    return nullptr;
}

CName GetInnerTypeName(const CBaseRTTIType* aType)
{
    if (const auto* type = GetInnerType(aType))
        return type->GetName();
    return {};
}
} // namespace Red
