#include "TweakTypeSpec.hpp"

namespace
{
constexpr auto ArrayPrefix = Red::GetTypePrefixStr<Red::DynArray>();
constexpr auto ArrayPrefixSize = ArrayPrefix.size() - 1;

using ResRefArrayType = Red::TypeLocator<Red::GetTypeName<Red::DynArray<Red::ResRef>>()>;
using ResRefType = Red::TypeLocator<Red::GetTypeName<Red::ResRef>()>;

using CNameArrayType = Red::TypeLocator<Red::GetTypeName<Red::DynArray<Red::CName>>()>;
using CNameType = Red::TypeLocator<Red::GetTypeName<Red::CName>()>;
} // namespace

namespace App
{
TweakTypeSpecPtr GetTweakTypeSpec(const std::string& aValue)
{
    using namespace Red::ERTDBFlatType;

    // Attempt to load a spec for non-foreign-key types
    if (auto spec = GetTweakTypeSpec(Red::CName(aValue.c_str())))
        return spec;

    // Attempt to look up foreign key arrays using shorthand syntax (e.g. "array:SomeType")
    if (aValue.starts_with(ArrayPrefix.data()) && aValue.length() > ArrayPrefixSize)
        return GetTweakTypeSpec(TweakDBIDArray, aValue.substr(ArrayPrefixSize));

    // Assume the name is a foreign key weak handle type using shorthand (e.g. "SomeType")
    return GetTweakTypeSpec(TweakDBID, aValue);
}

TweakTypeSpecPtr GetTweakTypeSpec(const char* aValue)
{
    return GetTweakTypeSpec(std::string(aValue));
}

TweakTypeSpecPtr GetTweakTypeSpec(Red::CName aName, const std::optional<std::string>& aForeignType)
{
    static Red::CRTTISystem* rtti = Red::CRTTISystem::Get();

    if (!Red::IsFlatType(aName))
        return nullptr;

    const auto isArray = Red::IsArrayType(aName);
    const auto isForeignKey = isArray ? Red::IsForeignKeyArray(aName) : Red::IsForeignKey(aName);
    const auto isResRef = isArray ? Red::IsResRefTokenArray(aName) : Red::IsResRefToken(aName);
    const auto isLocKey = isArray ? Red::IsLocKeyArray(aName) : Red::IsLocKey(aName);

    if (isForeignKey && !aForeignType.has_value())
        return nullptr;

    if (!isForeignKey && aForeignType.has_value())
        return nullptr;

    auto spec = Core::MakeShared<TweakTypeSpec>();

    spec->flatType = Red::GetFlatType(aName);
    spec->flatTypeName = aName;
    spec->propertyType = spec->flatType;
    spec->propertyTypeName = spec->flatTypeName;

    spec->isArray = isArray;
    spec->isForeignKey = isForeignKey;
    spec->isResRef = isResRef;
    spec->isLocKey = isLocKey;

    if (isForeignKey)
    {
        spec->foreignTypeName = Red::CNamePool::Add(Red::NormalizeRecordName(*aForeignType).c_str());
        spec->foreignType = rtti->GetClass(spec->foreignTypeName);
    }
    else if (isResRef)
    {
        spec->propertyType = isArray ? ResRefArrayType::Get() : ResRefType::Get();
        spec->propertyTypeName = spec->propertyType->GetName();
    }
    else if (isLocKey)
    {
        spec->propertyType = isArray ? CNameArrayType::Get() : CNameType::Get();
        spec->propertyTypeName = spec->propertyType->GetName();
    }

    return spec;
}
} // namespace App
