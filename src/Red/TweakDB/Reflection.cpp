#include "Reflection.hpp"

namespace
{
constexpr auto RecordTypePrefix = "gamedata";
constexpr auto RecordTypePrefixLength = std::char_traits<char>::length(RecordTypePrefix);
constexpr auto RecordTypeSuffix = "_Record";
constexpr auto RecordTypeSuffixLength = std::char_traits<char>::length(RecordTypeSuffix);

constexpr auto BaseRecordTypeName = Red::CName("gamedataTweakDBRecord");
constexpr auto ResRefTokenTypeName = Red::CName("redResourceReferenceScriptToken");
constexpr auto ResRefTokenArrayTypeName = Red::CName("array:redResourceReferenceScriptToken");

constexpr auto SchemaGroup = "RTDB.";
constexpr auto PropSeparator = ".";

constexpr auto DataOffsetSize = 3;
}

Red::TweakDBReflection::TweakDBReflection()
    : TweakDBReflection(Red::TweakDB::Get())
{
}

Red::TweakDBReflection::TweakDBReflection(Red::TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
    , m_rtti(Red::CRTTISystem::Get())
{
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::GetRecordInfo(const Red::CClass* aType)
{
    if (!IsRecordType(aType))
        return nullptr;

    auto iter = m_resolved.find(aType->GetName());

    if (iter != m_resolved.end())
        return iter->second.get();

    return CollectRecordInfo(aType).get();
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::GetRecordInfo(Red::CName aTypeName)
{
    auto iter = m_resolved.find(aTypeName);

    if (iter != m_resolved.end())
        return iter->second.get();

    return CollectRecordInfo(m_rtti->GetClass(aTypeName)).get();
}

Core::SharedPtr<Red::TweakDBRecordInfo> Red::TweakDBReflection::CollectRecordInfo(
    const Red::CClass* aType, Red::TweakDBID aSampleId)
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

    auto recordInfo = Core::MakeShared<Red::TweakDBRecordInfo>();
    recordInfo->name = aType->name;
    recordInfo->type = aType;
    recordInfo->typeHash = GetRecordTypeHash(aType);
    recordInfo->shortName = GetRecordShortName(aType->name);

    const auto parentInfo = CollectRecordInfo(aType->parent, sampleId);
    if (parentInfo)
    {
        recordInfo->parent = aType->parent;
        recordInfo->props.insert(parentInfo->props.begin(), parentInfo->props.end());
        recordInfo->extraFlats = parentInfo->extraFlats;
    }

    const auto baseOffset = aType->parent->size;

    for (uint32_t funcIndex = 0u; funcIndex < aType->funcs.size; ++funcIndex)
    {
        const auto func = aType->funcs[funcIndex];

        auto propName = ResolvePropertyName(sampleId, func->shortName);

        auto propInfo = Core::MakeShared<Red::TweakDBPropertyInfo>();
        propInfo->name = Red::CName(propName.c_str());
        propInfo->dataOffset = baseOffset + DataOffsetSize * recordInfo->props.size();

        // Case: Foreign Key Array => TweakDBID[]
        if (!func->returnType)
        {
            const auto arrayType = reinterpret_cast<Red::CRTTIArrayType*>(func->params[0]->type);
            const auto handleType = reinterpret_cast<Red::CRTTIWeakHandleType*>(arrayType->innerType);
            const auto recordType = reinterpret_cast<Red::CClass*>(handleType->innerType);

            propInfo->type = m_rtti->GetType(Red::ETweakDBFlatType::TweakDBIDArray);
            propInfo->isArray = true;
            propInfo->elementType = m_rtti->GetType(Red::ETweakDBFlatType::TweakDBID);
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

                propInfo->type = m_rtti->GetType(Red::ETweakDBFlatType::TweakDBID);
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
                    propInfo->type = m_rtti->GetType(Red::ETweakDBFlatType::ResourceArray);
                    propInfo->isArray = true;
                    propInfo->elementType = m_rtti->GetType(Red::ETweakDBFlatType::Resource);

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
                    propInfo->type = m_rtti->GetType(Red::ETweakDBFlatType::Resource);
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

    {
        auto extraFlatsIt = s_extraFlats.find(aType->name);
        if (extraFlatsIt != s_extraFlats.end())
        {
            recordInfo->extraFlats = true;

            for (const auto& extraFlat : extraFlatsIt.value())
            {
                auto propInfo = Core::MakeShared<Red::TweakDBPropertyInfo>();
                propInfo->name = Red::CName(extraFlat.appendix.c_str() + 1);
                propInfo->appendix = extraFlat.appendix;

                propInfo->type = m_rtti->GetType(extraFlat.typeName);
                propInfo->isArray = propInfo->type->GetType() == Red::ERTTIType::Array;
                propInfo->isForeignKey = !extraFlat.foreignTypeName.IsNone();

                if (propInfo->isArray)
                {
                    const auto arrayType = reinterpret_cast<const Red::CRTTIArrayType*>(propInfo->type);
                    propInfo->elementType = arrayType->innerType;
                }

                if (propInfo->isArray)
                {
                    propInfo->foreignType = m_rtti->GetClass(extraFlat.foreignTypeName);
                }

                propInfo->dataOffset = 0;
                propInfo->defaultValue = -1;

                recordInfo->props.insert({propInfo->name, propInfo});
            }
        }
    }

    for (auto& [_, propInfo] : recordInfo->props)
    {
        if (propInfo->dataOffset)
        {
            propInfo->defaultValue = ResolveDefaultValue(aType, propInfo->appendix);
        }
    }

    m_resolved.insert({ recordInfo->name, recordInfo });

    return recordInfo;
}

Red::TweakDBID Red::TweakDBReflection::GetRecordSampleId(const Red::CClass* aType)
{
    std::shared_lock<Red::SharedMutex> recordLockR(m_tweakDb->mutex01);
    auto* records = m_tweakDb->recordsByType.Get(const_cast<Red::CClass*>(aType));

    if (records == nullptr)
        return {};

    return records->Begin()->GetPtr<Red::TweakDBRecord>()->recordID;
}

uint32_t Red::TweakDBReflection::GetRecordTypeHash(const Red::CClass* aType)
{
    std::shared_lock<Red::SharedMutex> recordLockR(m_tweakDb->mutex01);
    auto* records = m_tweakDb->recordsByType.Get(const_cast<Red::CClass*>(aType));

    if (records == nullptr)
        return 0;

    return records->Begin()->GetPtr<Red::TweakDBRecord>()->GetTweakBaseHash();
}

std::string Red::TweakDBReflection::ResolvePropertyName(Red::TweakDBID aSampleId, Red::CName aGetterName)
{
    static const std::string dot = ".";

    std::string propName = aGetterName.ToString();
    propName[0] = static_cast<char>(std::tolower(propName[0]));

    auto propId = aSampleId + dot + propName;

    std::shared_lock<Red::SharedMutex> flatLockR(m_tweakDb->mutex00);

    auto propFlat = m_tweakDb->flats.Find(propId);
    if (propFlat == m_tweakDb->flats.End())
        propName[0] = static_cast<char>(std::toupper(propName[0]));

    return propName;
}

int32_t Red::TweakDBReflection::ResolveDefaultValue(const Red::CClass* aType, const std::string& aPropName)
{
    std::string defaultFlatName = SchemaGroup;

    defaultFlatName.append(GetRecordShortName(aType->GetName()));

    if (!aPropName.starts_with(PropSeparator))
    {
        defaultFlatName.append(PropSeparator);
    }

    defaultFlatName.append(aPropName);

    const auto defaultFlatId = Red::TweakDBID(defaultFlatName);

    std::shared_lock<Red::SharedMutex> flatLockR(m_tweakDb->mutex00);

    auto defaultFlat = m_tweakDb->flats.Find(defaultFlatId);

    if (defaultFlat == m_tweakDb->flats.End())
        return -1;

    return defaultFlat->ToTDBOffset();
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

bool Red::TweakDBReflection::IsFlatType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case Red::ETweakDBFlatType::Int:
    case Red::ETweakDBFlatType::Float:
    case Red::ETweakDBFlatType::Bool:
    case Red::ETweakDBFlatType::String:
    case Red::ETweakDBFlatType::CName:
    case Red::ETweakDBFlatType::TweakDBID:
    case Red::ETweakDBFlatType::LocKey:
    case Red::ETweakDBFlatType::Resource:
    case Red::ETweakDBFlatType::Quaternion:
    case Red::ETweakDBFlatType::EulerAngles:
    case Red::ETweakDBFlatType::Vector3:
    case Red::ETweakDBFlatType::Vector2:
    case Red::ETweakDBFlatType::Color:
    case Red::ETweakDBFlatType::IntArray:
    case Red::ETweakDBFlatType::FloatArray:
    case Red::ETweakDBFlatType::BoolArray:
    case Red::ETweakDBFlatType::StringArray:
    case Red::ETweakDBFlatType::CNameArray:
    case Red::ETweakDBFlatType::TweakDBIDArray:
    case Red::ETweakDBFlatType::LocKeyArray:
    case Red::ETweakDBFlatType::ResourceArray:
    case Red::ETweakDBFlatType::QuaternionArray:
    case Red::ETweakDBFlatType::EulerAnglesArray:
    case Red::ETweakDBFlatType::Vector3Array:
    case Red::ETweakDBFlatType::Vector2Array:
    case Red::ETweakDBFlatType::ColorArray:
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
    return IsRecordType(Red::CRTTISystem::Get()->GetClass(aTypeName));
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
    case Red::ETweakDBFlatType::IntArray:
    case Red::ETweakDBFlatType::FloatArray:
    case Red::ETweakDBFlatType::BoolArray:
    case Red::ETweakDBFlatType::StringArray:
    case Red::ETweakDBFlatType::CNameArray:
    case Red::ETweakDBFlatType::TweakDBIDArray:
    case Red::ETweakDBFlatType::LocKeyArray:
    case Red::ETweakDBFlatType::ResourceArray:
    case Red::ETweakDBFlatType::QuaternionArray:
    case Red::ETweakDBFlatType::EulerAnglesArray:
    case Red::ETweakDBFlatType::Vector3Array:
    case Red::ETweakDBFlatType::Vector2Array:
    case Red::ETweakDBFlatType::ColorArray:
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
    return aTypeName == Red::ETweakDBFlatType::TweakDBID;
}

bool Red::TweakDBReflection::IsForeignKey(const Red::CBaseRTTIType* aType)
{
    return aType && IsForeignKey(aType->GetName());
}

bool Red::TweakDBReflection::IsForeignKeyArray(Red::CName aTypeName)
{
    return aTypeName == Red::ETweakDBFlatType::TweakDBIDArray;
}

bool Red::TweakDBReflection::IsForeignKeyArray(const Red::CBaseRTTIType* aType)
{
    return aType && IsForeignKeyArray(aType->GetName());
}

bool Red::TweakDBReflection::IsResRefToken(Red::CName aTypeName)
{
    return aTypeName == ResRefTokenTypeName;
}

bool Red::TweakDBReflection::IsResRefToken(const Red::CBaseRTTIType* aType)
{
    return aType && IsResRefToken(aType->GetName());
}

bool Red::TweakDBReflection::IsResRefTokenArray(Red::CName aTypeName)
{
    return aTypeName == ResRefTokenArrayTypeName;
}

bool Red::TweakDBReflection::IsResRefTokenArray(const Red::CBaseRTTIType* aType)
{
    return aType && IsResRefTokenArray(aType->GetName());
}

Red::CName Red::TweakDBReflection::GetArrayType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case Red::ETweakDBFlatType::Int: return Red::ETweakDBFlatType::IntArray;
    case Red::ETweakDBFlatType::Float: return Red::ETweakDBFlatType::FloatArray;
    case Red::ETweakDBFlatType::Bool: return Red::ETweakDBFlatType::BoolArray;
    case Red::ETweakDBFlatType::String: return Red::ETweakDBFlatType::StringArray;
    case Red::ETweakDBFlatType::CName: return Red::ETweakDBFlatType::CNameArray;
    case Red::ETweakDBFlatType::TweakDBID: return Red::ETweakDBFlatType::TweakDBIDArray;
    case Red::ETweakDBFlatType::LocKey: return Red::ETweakDBFlatType::LocKeyArray;
    case Red::ETweakDBFlatType::Resource: return Red::ETweakDBFlatType::ResourceArray;
    case Red::ETweakDBFlatType::Quaternion: return Red::ETweakDBFlatType::QuaternionArray;
    case Red::ETweakDBFlatType::EulerAngles: return Red::ETweakDBFlatType::EulerAnglesArray;
    case Red::ETweakDBFlatType::Vector3: return Red::ETweakDBFlatType::Vector3Array;
    case Red::ETweakDBFlatType::Vector2: return Red::ETweakDBFlatType::Vector2Array;
    case Red::ETweakDBFlatType::Color: return Red::ETweakDBFlatType::ColorArray;
    }

    return {};
}

Red::CName Red::TweakDBReflection::GetArrayType(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetArrayType(aType->GetName());
}

Red::CName Red::TweakDBReflection::GetElementType(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case Red::ETweakDBFlatType::IntArray: return Red::ETweakDBFlatType::Int;
    case Red::ETweakDBFlatType::FloatArray: return Red::ETweakDBFlatType::Float;
    case Red::ETweakDBFlatType::BoolArray: return Red::ETweakDBFlatType::Bool;
    case Red::ETweakDBFlatType::StringArray: return Red::ETweakDBFlatType::String;
    case Red::ETweakDBFlatType::CNameArray: return Red::ETweakDBFlatType::CName;
    case Red::ETweakDBFlatType::TweakDBIDArray: return Red::ETweakDBFlatType::TweakDBID;
    case Red::ETweakDBFlatType::LocKeyArray: return Red::ETweakDBFlatType::LocKey;
    case Red::ETweakDBFlatType::ResourceArray: return Red::ETweakDBFlatType::Resource;
    case Red::ETweakDBFlatType::QuaternionArray: return Red::ETweakDBFlatType::Quaternion;
    case Red::ETweakDBFlatType::EulerAnglesArray: return Red::ETweakDBFlatType::EulerAngles;
    case Red::ETweakDBFlatType::Vector3Array: return Red::ETweakDBFlatType::Vector3;
    case Red::ETweakDBFlatType::Vector2Array: return Red::ETweakDBFlatType::Vector2;
    case Red::ETweakDBFlatType::ColorArray: return Red::ETweakDBFlatType::Color;
    }

    return {};
}

Red::CName Red::TweakDBReflection::GetElementType(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetElementType(aType->GetName());
}

Red::CName Red::TweakDBReflection::GetRecordFullName(Red::CName aName)
{
    return GetRecordFullName(aName.ToString());
}

Red::CName Red::TweakDBReflection::GetRecordFullName(const char* aName)
{
    std::string finalName = aName;

    if (!finalName.starts_with(RecordTypePrefix))
        finalName.insert(0, RecordTypePrefix);

    if (!finalName.ends_with(RecordTypeSuffix))
        finalName.append(RecordTypeSuffix);

    return finalName.c_str();
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

Core::SharedPtr<void> Red::TweakDBReflection::Construct(Red::CName aTypeName)
{
    switch (aTypeName)
    {
    case Red::ETweakDBFlatType::Int: return Core::MakeShared<int>();
    case Red::ETweakDBFlatType::Float: return Core::MakeShared<float>();
    case Red::ETweakDBFlatType::Bool: return Core::MakeShared<bool>();
    case Red::ETweakDBFlatType::String: return Core::MakeShared<Red::CString>();
    case Red::ETweakDBFlatType::CName: return Core::MakeShared<Red::CName>();
    case Red::ETweakDBFlatType::TweakDBID: return Core::MakeShared<Red::TweakDBID>();
    case Red::ETweakDBFlatType::LocKey: return Core::MakeShared<Red::LocKeyWrapper>();
    case Red::ETweakDBFlatType::Resource: return Core::MakeShared<Red::ResourceAsyncReference<>>();
    case Red::ETweakDBFlatType::Quaternion: return Core::MakeShared<Red::Quaternion>();
    case Red::ETweakDBFlatType::EulerAngles: return Core::MakeShared<Red::EulerAngles>();
    case Red::ETweakDBFlatType::Vector3: return Core::MakeShared<Red::Vector3>();
    case Red::ETweakDBFlatType::Vector2: return Core::MakeShared<Red::Vector2>();
    case Red::ETweakDBFlatType::Color: return Core::MakeShared<Red::Color>();
    case Red::ETweakDBFlatType::IntArray: return Core::MakeShared<Red::DynArray<int>>();
    case Red::ETweakDBFlatType::FloatArray: return Core::MakeShared<Red::DynArray<float>>();
    case Red::ETweakDBFlatType::BoolArray: return Core::MakeShared<Red::DynArray<bool>>();
    case Red::ETweakDBFlatType::StringArray: return Core::MakeShared<Red::DynArray<Red::CString>>();
    case Red::ETweakDBFlatType::CNameArray: return Core::MakeShared<Red::DynArray<Red::CName>>();
    case Red::ETweakDBFlatType::TweakDBIDArray: return Core::MakeShared<Red::DynArray<Red::TweakDBID>>();
    case Red::ETweakDBFlatType::LocKeyArray: return Core::MakeShared<Red::DynArray<Red::LocKeyWrapper>>();
    case Red::ETweakDBFlatType::ResourceArray: return Core::MakeShared<Red::DynArray<Red::ResourceAsyncReference<>>>();
    case Red::ETweakDBFlatType::QuaternionArray: return Core::MakeShared<Red::DynArray<Red::Quaternion>>();
    case Red::ETweakDBFlatType::EulerAnglesArray: return Core::MakeShared<Red::DynArray<Red::EulerAngles>>();
    case Red::ETweakDBFlatType::Vector3Array: return Core::MakeShared<Red::DynArray<Red::Vector3>>();
    case Red::ETweakDBFlatType::Vector2Array: return Core::MakeShared<Red::DynArray<Red::Vector2>>();
    case Red::ETweakDBFlatType::ColorArray: return Core::MakeShared<Red::DynArray<Red::Color>>();
    }

    return {};
}

Core::SharedPtr<void> Red::TweakDBReflection::Construct(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return Construct(aType->GetName());
}

Core::SharedPtr<void> Red::TweakDBReflection::Clone(Red::CName aTypeName, void* aValue)
{
    switch (aTypeName)
    {
    case Red::ETweakDBFlatType::Int: return Core::MakeShared<int>(*reinterpret_cast<int*>(aValue));
    case Red::ETweakDBFlatType::Float: return Core::MakeShared<float>(*reinterpret_cast<float*>(aValue));
    case Red::ETweakDBFlatType::Bool: return Core::MakeShared<bool>(*reinterpret_cast<bool*>(aValue));
    case Red::ETweakDBFlatType::String: return Core::MakeShared<Red::CString>(*reinterpret_cast<Red::CString*>(aValue));
    case Red::ETweakDBFlatType::CName: return Core::MakeShared<Red::CName>(*reinterpret_cast<Red::CName*>(aValue));
    case Red::ETweakDBFlatType::TweakDBID: return Core::MakeShared<Red::TweakDBID>(*reinterpret_cast<Red::TweakDBID*>(aValue));
    case Red::ETweakDBFlatType::LocKey: return Core::MakeShared<Red::gamedataLocKeyWrapper>(*reinterpret_cast<Red::gamedataLocKeyWrapper*>(aValue));
    case Red::ETweakDBFlatType::Resource: return Core::MakeShared<Red::ResourceAsyncReference<>>(*reinterpret_cast<Red::ResourceAsyncReference<>*>(aValue));
    case Red::ETweakDBFlatType::Quaternion: return Core::MakeShared<Red::Quaternion>(*reinterpret_cast<Red::Quaternion*>(aValue));
    case Red::ETweakDBFlatType::EulerAngles: return Core::MakeShared<Red::EulerAngles>(*reinterpret_cast<Red::EulerAngles*>(aValue));
    case Red::ETweakDBFlatType::Vector3: return Core::MakeShared<Red::Vector3>(*reinterpret_cast<Red::Vector3*>(aValue));
    case Red::ETweakDBFlatType::Vector2: return Core::MakeShared<Red::Vector2>(*reinterpret_cast<Red::Vector2*>(aValue));
    case Red::ETweakDBFlatType::Color: return Core::MakeShared<Red::Color>(*reinterpret_cast<Red::Color*>(aValue));
    }

    return {};
}

Core::SharedPtr<void> Red::TweakDBReflection::Clone(const Red::CBaseRTTIType* aType, void* aValue)
{
    if (!aType)
        return {};

    return Clone(aType->GetName(), aValue);
}

Red::TweakDB* Red::TweakDBReflection::GetTweakDB()
{
    return m_tweakDb;
}
