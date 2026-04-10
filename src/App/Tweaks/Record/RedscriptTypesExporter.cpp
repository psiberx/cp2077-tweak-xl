#include "RedscriptTypesExporter.hpp"

namespace App
{
RedscriptTypesExporter::RedscriptTypesExporter(const Core::SharedPtr<ScriptableRecordManager>& aRecordManager)
    : m_recordManager(aRecordManager)
{
    m_env.set_lstrip_blocks(true);
    m_env.set_trim_blocks(true);

    try
    {
        m_baseTemplate = m_env.parse(BaseRecordTemplate);
        m_recordTemplate = m_env.parse(ScriptableRecordTemplate);
    }
    catch (const std::exception& e)
    {
        LogError("Failed to parse RedScript template: {}", e.what());
    }
}

void RedscriptTypesExporter::ExportRedscriptTypes(const std::filesystem::path& aDir)
{
    try
    {
        std::string content = m_env.render(m_baseTemplate, {});

        for (const auto& spec : m_recordManager->GetRecordSpecs())
        {
            if (const auto json = ToJson(spec); !json.empty())
            {
                content += m_env.render(m_recordTemplate, json);
            }
        }

        const auto filePath = aDir / "TweakXL.Records.reds";

        std::ofstream file(filePath, std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Failed to open output file: " + filePath.string());

        file.write(content.data(), static_cast<std::streamsize>(content.size()));
        if (!file.good())
            throw std::runtime_error("Failed to write output file: " + filePath.string());

        file.flush();
        file.close();

        LogInfo("Exported RedScript scriptable record class definitions to: {}", filePath.string());
    }
    catch (const std::exception& e)
    {
        LogError("Failed to export RedScript types: {}", e.what());
    }
}

RedscriptTypesExporter::Json RedscriptTypesExporter::ToJson(const ScriptableRecordSpecPtr& aSpec) const
{
    if (!aSpec->isDescribed)
        return nullptr;

    Json json;

    json["TypeName"] = aSpec->aliasName;
    json["Parent"] = aSpec->parent ? *aSpec->parent : "ScriptableTweakDBRecord";
    json["Properties"] = Json::array();

    for (const auto& prop : aSpec->props | std::views::values)
    {
        if (auto property = ToJson(prop); !property.empty())
            json["Properties"].push_back(property);
    }

    return json;
}

RedscriptTypesExporter::Json RedscriptTypesExporter::ToJson(const ScriptablePropertySpecPtr& aSpec) const
{
    if (!aSpec->isCreated)
        return nullptr;

    Json json;

    json["Name"] = aSpec->functionName;
    json["Type"] = aSpec->typeSpec->propertyTypeName.ToString();
    json["IsForeignKey"] = aSpec->typeSpec->isForeignKey;
    json["IsArray"] = aSpec->typeSpec->isArray;
    json["IsResRef"] = aSpec->typeSpec->isResRef;
    json["IsLocKey"] = aSpec->typeSpec->isResRef;
    json["ForeignType"] = GetClassScriptName(aSpec->typeSpec->foreignType);
    json["ElementType"] = Red::GetElementTypeName(aSpec->typeSpec->propertyType).ToString();

    return json;
}

std::string RedscriptTypesExporter::GetClassScriptName(const Red::CClass* aClass) const
{
    static auto* rtti = RED4ext::CRTTISystem::Get();

    if (aClass)
        return rtti->ConvertNativeToScriptName(aClass->GetName()).ToString();
    return "";
}
} // namespace App
