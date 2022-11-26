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

constexpr auto SchemaPackage = "RTDB.";
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

    {
        std::shared_lock lockR(m_mutex);
        auto iter = m_resolved.find(aType->GetName());
        if (iter != m_resolved.end())
            return iter->second.get();
    }

    return CollectRecordInfo(aType).get();
}

const Red::TweakDBRecordInfo* Red::TweakDBReflection::GetRecordInfo(Red::CName aTypeName)
{
    {
        std::shared_lock lockR(m_mutex);
        auto iter = m_resolved.find(aTypeName);
        if (iter != m_resolved.end())
            return iter->second.get();
    }

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

    auto recordInfo = Red::MakeInstance<Red::TweakDBRecordInfo>();
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

        auto propInfo = Red::MakeInstance<Red::TweakDBPropertyInfo>();
        propInfo->name = Red::CName(propName.c_str());
        propInfo->dataOffset = baseOffset + DataOffsetSize * recordInfo->props.size();

        // Case: Foreign Key Array => TweakDBID[]
        if (!func->returnType)
        {
            const auto arrayType = reinterpret_cast<Red::CRTTIArrayType*>(func->params[0]->type);
            const auto handleType = reinterpret_cast<Red::CRTTIWeakHandleType*>(arrayType->innerType);
            const auto recordType = reinterpret_cast<Red::CClass*>(handleType->innerType);

            propInfo->type = m_rtti->GetType(Red::ERTDBFlatType::TweakDBIDArray);
            propInfo->isArray = true;
            propInfo->elementType = m_rtti->GetType(Red::ERTDBFlatType::TweakDBID);
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

                propInfo->type = m_rtti->GetType(Red::ERTDBFlatType::TweakDBID);
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
                    propInfo->type = m_rtti->GetType(Red::ERTDBFlatType::ResRefArray);
                    propInfo->isArray = true;
                    propInfo->elementType = m_rtti->GetType(Red::ERTDBFlatType::ResRef);

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
                    propInfo->type = m_rtti->GetType(Red::ERTDBFlatType::ResRef);
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

        recordInfo->props[propInfo->name] = propInfo;
    }

    {
        auto extraFlatsIt = s_extraFlats.find(aType->name);
        if (extraFlatsIt != s_extraFlats.end())
        {
            recordInfo->extraFlats = true;

            for (const auto& extraFlat : extraFlatsIt.value())
            {
                auto propInfo = Red::MakeInstance<Red::TweakDBPropertyInfo>();
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

                recordInfo->props[propInfo->name] = propInfo;
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

    {
        std::unique_lock lockRW(m_mutex);
        m_resolved.insert({ recordInfo->name, recordInfo });
    }

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
    std::string defaultFlatName = SchemaPackage;

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

Red::CName Red::TweakDBReflection::GetArrayTypeName(Red::CName aTypeName)
{
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
    }

    return {};
}

Red::CName Red::TweakDBReflection::GetArrayTypeName(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetArrayTypeName(aType->GetName());
}

Red::CName Red::TweakDBReflection::GetElementTypeName(Red::CName aTypeName)
{
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
    }

    return {};
}

Red::CName Red::TweakDBReflection::GetElementTypeName(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return GetElementTypeName(aType->GetName());
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

Red::InstancePtr<> Red::TweakDBReflection::Construct(Red::CName aTypeName)
{
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
    }

    return {};
}

Red::InstancePtr<> Red::TweakDBReflection::Construct(const Red::CBaseRTTIType* aType)
{
    if (!aType)
        return {};

    return Construct(aType->GetName());
}

Red::TweakDB* Red::TweakDBReflection::GetTweakDB()
{
    return m_tweakDb;
}
