#include "SchemaChangeset.hpp"

#include <regex>

namespace
{
struct VectorDiff
{
    std::vector<Red::CName> removed;
    std::vector<Red::CName> added;
    std::vector<Red::CName> existing;
};

template<typename T>
std::vector<Red::CName> GetKeys(Core::Map<Red::CName, T>& aMap)
{
    std::vector<Red::CName> keys;
    for (auto& key : aMap | std::views::keys)
        keys.push_back(key);
    return keys;
}

VectorDiff GetVectorDiff(std::vector<Red::CName> aLhs, std::vector<Red::CName> aRhs)
{
    std::ranges::sort(aLhs);
    std::ranges::sort(aRhs);

    VectorDiff diff;

    std::ranges::set_difference(aLhs, aRhs, std::back_inserter(diff.added));
    std::ranges::set_difference(aRhs, aLhs, std::back_inserter(diff.removed));
    std::ranges::set_intersection(aLhs, aRhs, std::back_inserter(diff.existing));

    return diff;
}

const auto NameRegex = std::regex("^[a-zA-Z_][a-zA-Z0-9_]*$");

} // namespace

namespace App
{
bool SchemaChangeset::MakeRecord(const std::string& aName, const std::optional<std::string>& aParent)
{
    if (!std::regex_match(aName, NameRegex))
    {
        LogError(R"(Invalid scriptable record name "{}". Record names must start with a letter or underscore and can )"
                 "only contain letters, digits, and underscores.",
                 aName);
        return false;
    }

    const auto name = Red::NormalizeRecordName(aName);
    const auto aliasName = Red::GetRecordAliasName<std::string>(name);

    if (name.empty())
    {
        LogError(R"(Invalid scriptable record name "{}".)", aliasName);
        return false;
    }

    Red::CName cname{name.c_str()};

    if (m_pendingRecords.contains(cname))
    {
        LogError(R"(Record "{}" is already defined; skipping...)", aliasName);
        return false;
    }

    if (aliasName == aName)
    {
        LogInfo(R"(Record name "{}" is normalized to "{}".)", aName, aliasName);
    }

    m_pendingRecords.insert({cname, {name, aParent}});

    return true;
}

bool SchemaChangeset::MakeProperty(const std::string& aRecordName, const std::string& aPropName,
                                   const TweakTypeSpecPtr& aTypeInfo, const Red::InstancePtr<>& aDefaultValue)
{
    if (!std::regex_match(aPropName, NameRegex))
    {
        LogError(R"(Invalid scriptable property name "{}" for record "{}". Property names must start with a letter )"
                 "or underscore and can only contain letters, digits, and underscores.",
                 aPropName, aRecordName);
        return false;
    }

    const auto recordName = Red::NormalizeRecordName(aRecordName);

    if (recordName.empty())
    {
        LogError(R"(Invalid scriptable record name "{}" for property "{}".)", aRecordName, aPropName);
        return false;
    }

    const auto aliasName = Red::GetRecordAliasName<std::string>(recordName);
    const Red::CName recordCName{recordName.c_str()};

    if (!m_pendingRecords.contains(recordCName))
    {
        LogError(R"(Cannot define property "{}" for record "{}", the record is not defined.)", aPropName, aliasName);
        return false;
    }

    auto& record = m_pendingRecords[recordCName];

    const Red::CName propertyCName{aPropName.c_str()};

    if (record.properties.contains(propertyCName))
    {
        LogError(R"(Property "{}" of record "{}" is already defined; skipping...)", aPropName, aliasName);
        return false;
    }

    PropertyEntry entry;
    entry.name = aPropName;
    entry.type = aTypeInfo;
    entry.defaultValue = aDefaultValue;

    record.properties.insert({propertyCName, entry});

    return true;
}

void SchemaChangeset::Commit(const Core::SharedPtr<ScriptableRecordManager>& aRecordManager)
{
    if (!aRecordManager)
    {
        LogError("Cannot commit schema changeset: ScriptableRecordManager is not available.");
        return;
    }

    auto recordDiff = GetVectorDiff(GetKeys(m_pendingRecords), aRecordManager->GetRecordSpecNames());

    for (const auto& name : recordDiff.added)
    {
        auto& recordEntry = m_pendingRecords[name];

        const auto recordSpec = aRecordManager->RegisterScriptableRecordType(recordEntry.name, recordEntry.parent);

        if (!recordSpec)
            continue;

        for (const auto& property : recordEntry.properties | std::views::values)
            aRecordManager->RegisterScriptableProperty(recordSpec, property.name, property.type, property.defaultValue);
    }

    for (const auto name : recordDiff.removed)
        aRecordManager->UnregisterScriptableRecordType(name);

    for (const auto name : recordDiff.existing)
    {
        auto recordSpec = aRecordManager->GetRecordSpec(name);
        auto recordEntry = m_pendingRecords[name];

        auto propsDiff = GetVectorDiff(GetKeys(recordEntry.properties), GetKeys(recordSpec->props));

        for (const auto& propName : propsDiff.added)
        {
            const auto& propEntry = recordEntry.properties.at(propName);
            aRecordManager->RegisterScriptableProperty(recordSpec, propEntry.name, propEntry.type,
                                                       propEntry.defaultValue);
        }

        for (const auto& propName : propsDiff.removed)
        {
            const auto& propEntry = recordEntry.properties.at(propName);
            aRecordManager->UnregisterScriptableProperty(recordSpec, propEntry.name);
        }

        for (const auto& propName : propsDiff.existing)
        {
            auto& propEntry = recordEntry.properties.at(propName);
            auto propSpec = aRecordManager->GetPropertySpec(name, propName);

            if (*propSpec->typeSpec != *propEntry.type)
            {
                LogError(R"({}: Type "{}" for property "{}" is not compatible with the existing type "{}".)",
                         recordEntry.name, propEntry.type->flatTypeName.ToString(), propEntry.name,
                         propSpec->typeSpec->flatTypeName.ToString());
                continue;
            }

            if (IsEqual(propEntry.type->flatType, propEntry.defaultValue, propSpec->defaultValue))
            {
                continue;
            }

            aRecordManager->UpdateScriptableProperty(recordSpec, propEntry.name, propEntry.defaultValue);
        }
    }

    const auto specs = aRecordManager->GetRecordSpecs();

    for (auto& spec : specs)
        aRecordManager->RegisterRTTIType(spec);

    for (auto& recordSpec : specs)
    {
        if (!recordSpec->isDescribed)
        {
            if (!aRecordManager->DescribeRTTIType(recordSpec))
                continue;
        }

        for (auto& propSpec : recordSpec->props | std::views::values)
            aRecordManager->CreatePropertyFunctions(recordSpec, propSpec);
    }

    if (aRecordManager->IsTweakDBReady())
        aRecordManager->InsertDefaultValues();
}

bool SchemaChangeset::IsEmpty() const
{
    return m_pendingRecords.empty();
}

bool SchemaChangeset::IsEqual(const Red::CBaseRTTIType* aType, const Red::InstancePtr<>& aLhs,
                              const Red::InstancePtr<>& aRhs)
{
    if (!aType)
        return false;

    if (!aLhs && !aRhs)
        return true;

    if (!aLhs || !aRhs)
        return false;

    if (aType->GetType() == Red::ERTTIType::Array)
    {
        auto* arrayType = Red::ToArrayType(aType);
        auto* innerType = arrayType->GetInnerType();

        if (arrayType->GetLength(aLhs.get()) != arrayType->GetLength(aRhs.get()))
            return false;

        for (size_t i = 0; i < arrayType->GetLength(aLhs.get()); ++i)
        {
            if (!innerType->IsEqual(arrayType->GetElement(aLhs.get(), i), arrayType->GetElement(aRhs.get(), i)))
                return false;
        }

        return true;
    }

    return const_cast<Red::CBaseRTTIType*>(aType)->IsEqual(aLhs.get(), aRhs.get());
}
} // namespace App
