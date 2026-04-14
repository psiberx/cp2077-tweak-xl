#include "RecordInfo.hpp"

#include "Red/TweakDB/Reflection.hpp"

namespace Red
{

static constexpr std::string_view NameSeparator = ".";

void TweakDBPropertyInfo::SetName(const char* aName)
{
    SetName(std::string(aName));
}

void TweakDBPropertyInfo::SetName(const std::string& aName)
{
    m_name = CNamePool::Add(aName.c_str());
    m_appendix = std::string(NameSeparator).append(aName);
}

void TweakDBPropertyInfo::SetType(const CBaseRTTIType* aType)
{
    m_type = aType;
}

void TweakDBPropertyInfo::SetElementType(const CBaseRTTIType* aElementType)
{
    m_elementType = aElementType;
}

void TweakDBPropertyInfo::SetForeignType(const CClass* aForeignType)
{
    m_foreignType = aForeignType;
}

void TweakDBPropertyInfo::SetArray(bool aIsArray)
{
    m_isArray = aIsArray;
}

void TweakDBPropertyInfo::SetForeignKey(bool aIsForeignKey)
{
    m_isForeignKey = aIsForeignKey;
}

void TweakDBPropertyInfo::SetDataOffset(uintptr_t aDataOffset)
{
    SetDataOffset(aDataOffset > 0 ? std::optional{aDataOffset} : std::nullopt);
}

void TweakDBPropertyInfo::SetDataOffset(const std::optional<uintptr_t> aDataOffset)
{
    m_dataOffset = aDataOffset > 0 ? std::optional{aDataOffset} : std::nullopt;
}

void TweakDBPropertyInfo::SetDefaultValue(int32_t aDefaultValue)
{
    SetDefaultValue(aDefaultValue > 0 ? std::optional{aDefaultValue} : std::nullopt);
}

void TweakDBPropertyInfo::SetDefaultValue(const std::optional<int32_t> aDefaultValue)
{
    m_defaultValue = aDefaultValue;
}

CName TweakDBPropertyInfo::GetName() const
{
    return m_name;
}

const CBaseRTTIType* TweakDBPropertyInfo::GetType() const
{
    return m_type;
}

const CBaseRTTIType* TweakDBPropertyInfo::GetElementType() const
{
    return m_elementType;
}

const CClass* TweakDBPropertyInfo::GetForeignType() const
{
    return m_foreignType;
}

bool TweakDBPropertyInfo::IsArray() const
{
    return m_isArray;
}

bool TweakDBPropertyInfo::IsForeignKey() const
{
    return m_isForeignKey;
}

const std::string& TweakDBPropertyInfo::GetAppendix() const
{
    return m_appendix;
}

std::optional<uintptr_t> TweakDBPropertyInfo::GetDataOffset() const
{
    return m_dataOffset;
}

std::optional<int32_t> TweakDBPropertyInfo::GetDefaultValue() const
{
    return m_defaultValue;
}

bool TweakDBPropertyInfo::IsValid() const
{
    if (m_name.IsNone() || !m_type || !TweakDBReflection::IsFlatType(m_type) || m_appendix.length() < 2 ||
        !m_appendix.starts_with(NameSeparator))
    {
        return false;
    }

    switch (m_type->GetName())
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
        return !m_isArray && !m_elementType && !m_isForeignKey && !m_foreignType;
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
        return m_isArray && m_elementType && !m_isForeignKey && !m_foreignType &&
               m_elementType->GetName() == TweakDBReflection::GetElementTypeName(m_type);
    case ERTDBFlatType::TweakDBID:
        return !m_isArray && !m_elementType && m_isForeignKey && m_foreignType;
    case ERTDBFlatType::TweakDBIDArray:
        return m_isArray && m_elementType && m_isForeignKey && m_foreignType &&
               m_elementType->GetName() == ERTDBFlatType::TweakDBID;
    default:
        return false;
    }
}

void TweakDBRecordInfo::SetName(const char* aName)
{
    m_name = TweakDBReflection::GetRecordFullName<CName>(aName);
    m_aliasName = TweakDBReflection::GetRecordAliasName<CName>(aName);
    m_shortName = TweakDBReflection::GetRecordShortName<std::string>(aName);
    m_typeHash = TweakDBReflection::GetRecordTypeHash(m_shortName);
}

void TweakDBRecordInfo::SetName(CName aName)
{
    SetName(aName.ToString());
}

void TweakDBRecordInfo::SetType(const CClass* aType)
{
    m_type = aType;
}

void TweakDBRecordInfo::SetParent(const CClass* aParent)
{
    m_parent = aParent;
}

Core::SharedPtr<const TweakDBPropertyInfo> TweakDBRecordInfo::AddProperty(
    Core::SharedPtr<TweakDBPropertyInfo> aProperty)
{
    if (!aProperty->IsValid())
    {
        return nullptr;
    }

    m_props[aProperty->GetName()] = aProperty;
    return aProperty;
}

CName TweakDBRecordInfo::GetName() const
{
    return m_name;
}

CName TweakDBRecordInfo::GetAliasName() const
{
    return m_aliasName;
}

const std::string& TweakDBRecordInfo::GetShortName() const
{
    return m_shortName;
}

const CClass* TweakDBRecordInfo::GetType() const
{
    return m_type;
}

const CClass* TweakDBRecordInfo::GetParent() const
{
    return m_parent;
}

uint32_t TweakDBRecordInfo::GetTypeHash() const
{
    return m_typeHash;
}

Core::SharedPtr<const TweakDBPropertyInfo> TweakDBRecordInfo::GetProperty(CName aPropName) const
{
    const auto& propIt = m_props.find(aPropName);
    return propIt != m_props.end() ? propIt->second : nullptr;
}

const Core::Map<CName, Core::SharedPtr<const TweakDBPropertyInfo>>& TweakDBRecordInfo::GetProperties() const
{
    return m_props;
}

bool TweakDBRecordInfo::IsValid() const
{
    if (m_name.IsNone() || m_aliasName.IsNone() || m_shortName.empty() || !m_type || m_type->GetName() != m_name)
    {
        return false;
    }

    if (m_parent && !TweakDBReflection::IsRecordType(m_parent))
    {
        return false;
    }

    return true;
}

TweakDBRecordInfo& TweakDBRecordInfo::operator+=(const TweakDBRecordInfo* aOther)
{
    return operator+=(*aOther);
}

TweakDBRecordInfo& TweakDBRecordInfo::operator+=(const TweakDBRecordInfo& aOther)
{
    assert(m_type->parent == aOther.m_type);

    m_parent = m_type->parent;
    m_props.insert(aOther.m_props.begin(), aOther.m_props.end());

    return *this;
}

} // namespace Red
