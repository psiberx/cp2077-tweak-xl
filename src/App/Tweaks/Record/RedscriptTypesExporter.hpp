#pragma once

#include "ScriptableRecordManager.hpp"
#include <filesystem>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

namespace App
{
/**
 * @brief The template for generating the base RedScript class definition for scriptable TweakDB records, which all
 * generated scriptable record classes will inherit from.
 */
static auto BaseRecordTemplate = R"RS(
public abstract native class ScriptableTweakDBRecord extends TweakDBRecord {
}
)RS";

/**
 * @brief The template for generating RedScript class definitions for scriptable TweakDB records and functions for
 * interacting with their properties. The generated functions vary based on the property's characteristics, such as
 * whether it is an array, a foreign key, etc.
 */
static auto ScriptableRecordTemplate = R"RS(
public native class {{ TypeName }} extends {{ Parent }} {
    {% for prop in Properties %}
    {% if prop.IsArray and prop.IsForeignKey %}
    public func {{ prop.Name }}(out outList: array<wref<{{ prop.ForeignType }}>>) {}
    public func Get{{ prop.Name }}Count() -> Int32 {}
    public func Get{{ prop.Name }}Item(index: Int32) -> wref<{{ prop.ForeignType }}> {}
    public func Get{{ prop.Name }}ItemHandle(index: Int32) -> ref<{{ prop.ForeignType }}> {}
    public func {{ prop.Name }}Contains(item: wref<{{ prop.ForeignType }}>) -> Bool {}
    {% else if prop.IsForeignKey %}
    public func {{ prop.Name }}() -> wref<{{ prop.ForeignType }}> {}
    public func {{ prop.Name }}Handle() -> ref<{{ prop.ForeignType }}> {}
    {% else if prop.IsArray and prop.IsResRef %}
    public func {{ prop.Name }}() -> array<ResRef> {}
    public func Get{{ prop.Name }}Count() -> Int32 {}
    public func Get{{ prop.Name }}Item(index: Int32) -> ResRef {}
    {% else if prop.IsArray %}
    public func {{ prop.Name }}() -> array<{{ prop.ElementType }}> {}
    public func Get{{ prop.Name }}Count() -> Int32 {}
    public func Get{{ prop.Name }}Item(index: Int32) -> {{ prop.ElementType }} {}
    public func {{ prop.Name }}Contains(item: {{ prop.ElementType }}) -> Bool {}
    {% else if prop.IsResRef %}
    public func {{ prop.Name }}() -> ResRef {}
    {% else %}
    public func {{ prop.Name }}() -> {{ prop.Type }} {}
    {% endif %}
    {% endfor %}
}
)RS";

/**
 * @brief Provides functionality to export RedScript class and property definitions for scriptable TweakDB records. This
 * ensures that definitions are consistent with TweakXL and avoids requiring mod authors to define the definitions
 * manually.
 *
 * Only scriptable TweakDB records and properties that have been successfully created will be exposed via RedScript.
 */
class RedscriptTypesExporter : Core::LoggingAgent
{
public:
    /**
     * @brief Alias for the JSON type from the nlohmann/json library.
     */
    using Json = nlohmann::json;

    /**
     * @brief Constructs a new RedScriptTypesExporter instance with the given shared pointer to a
     * ScriptableRecordManager. The ScriptableRecordManager is used to access scriptable record specs and their
     * properties for RedScript code generation.
     *
     * @param aRecordManager A shared pointer to the ScriptableRecordManager instance used to access scriptable record
     * specs and their properties for RedScript code generation.
     */
    explicit RedscriptTypesExporter(const Core::SharedPtr<ScriptableRecordManager>& aRecordManager);

    /**
     * @brief Exports RedScript class and property definitions for the given vector of scriptable record specs to a file
     * at the specified path. Only specs that have been successfully described will be exported.
     *
     * @param aDir The path to the file where the RedScript definitions should be exported. If the file already
     * exists, it will be overwritten.
     * @return true if the export was successful, false otherwise.
     */
    void ExportRedscriptTypes(const std::filesystem::path& aDir);

private:
    /**
     * @brief Converts the given scriptable record spec to a JSON representation that can be used for RedScript code
     * generation.
     *
     * @param aSpec The scriptable record spec to convert.
     * @return A JSON object containing the converted scriptable record spec, or nullptr if the spec is not described.
     */
    [[nodiscard]] Json ToJson(const ScriptableRecordSpecPtr& aSpec) const;

    /**
     * @brief Converts the given scriptable property spec to a JSON representation that can be used for RedScript code
     * generation.
     *
     * @param aSpec The scriptable property spec to convert.
     * @return A JSON object containing the converted scriptable property spec, or nullptr if the spec is not described.
     */
    [[nodiscard]] Json ToJson(const ScriptablePropertySpecPtr& aSpec) const;

    /**
     * @brief Retrieves the script name of a given scriptable record class, which is used for RedScript code generation.
     *
     * The script name is derived from the class's name and is used as the name of the generated RedScript class
     * definition for the record type.
     *
     * @param aClass The scriptable record class for which to retrieve the script name. This should be a valid class
     * corresponding to a described scriptable record spec.
     * @return The script name of the given scriptable record class, which is used for RedScript code generation.
     */
    std::string GetClassScriptName(const Red::CClass* aClass) const;

    /**
     * @brief The inja environment used for rendering the RedScript code from the template.
     */
    inja::Environment m_env;

    /**
     * @brief The parsed inja template used for declaring the @c ScriptableTweakDBRecord type.
     */
    inja::Template m_baseTemplate;

    /**
     * @brief The parsed inja template used for declaring a scriptable record type and its property functions.
     */
    inja::Template m_recordTemplate;

    /**
     * @brief A shared pointer to the ScriptableRecordManager instance used to access scriptable record specs and their
     * properties for RedScript code generation.
     */
    Core::SharedPtr<ScriptableRecordManager> m_recordManager;
};
} // namespace App
