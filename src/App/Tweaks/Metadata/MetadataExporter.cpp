#include "MetadataExporter.hpp"
#include "App/Tweaks/Declarative/Red/RedReader.hpp"
#include "Red/TweakDB/Manager.hpp"
#include "Red/TweakDB/Source/Parser.hpp"

namespace
{
constexpr auto TweakExtension = L".tweak";
constexpr auto SchemaPackage = Red::TweakSchemaPackage;
constexpr auto NameSeparator = Red::TweakGrammar::Name::Separator;
constexpr auto InlineSuffix = "_inline";
}

App::MetadataExporter::MetadataExporter(Core::SharedPtr<Red::TweakDBManager> aManager)
    : m_manager(std::move(aManager))
    , m_resolved(true)
{
}

void App::MetadataExporter::LoadSource(const std::filesystem::path& aSourceDir)
{
    for (const auto& entry : std::filesystem::recursive_directory_iterator(aSourceDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == TweakExtension)
        {
            m_sources.push_back(Red::TweakParser::Parse(entry.path()));
            m_resolved = false;
        }
    }
}

void App::MetadataExporter::ResolveGroups()
{
    if (m_resolved)
        return;

    m_groups.clear();

    Core::Map<std::string, Core::Map<std::string, Red::TweakGroupPtr>> map;

    for (auto& source : m_sources)
    {
        for (auto& group : source->groups)
        {
            map[source->package][group->name] = group;

            if (source->isPackage)
            {
                group->name = source->package + NameSeparator + group->name;
            }

            m_groups[group->name] = group;

            auto counter = 0;
            for (auto& inlined : group->inlines)
            {
                map[source->package][inlined->group->name] = inlined->group;

                inlined->group->name = group->name + InlineSuffix + std::to_string(counter++);

                m_groups[inlined->group->name] = inlined->group;
            }
        }
    }

    for (auto& source : m_sources)
    {
        Core::Vector<Red::TweakGroupPtr> groups;
        groups.reserve(source->groups.size() + source->inlines.size());
        groups.insert(groups.end(), source->groups.begin(), source->groups.end());
        for (auto& inlined : source->inlines)
        {
            groups.push_back(inlined->group);
        }

        for (auto& group : groups)
        {
            if (group->base.empty() || m_groups.contains(group->base))
                continue;

            if (map[source->package].contains(group->base))
            {
                group->base = source->package + NameSeparator + group->base;
            }
            else
            {
                for (auto& package : source->usings)
                {
                    if (map[package].contains(group->base))
                    {
                        group->base = package + NameSeparator + group->base;
                        break;
                    }
                }
            }
        }
    }

    for (auto& [_, group] : m_groups)
    {
        if (group->base.empty() || group->isSchema || group->isQuery)
            continue;

        auto parent = m_groups[group->base];
        while (parent)
        {
            if (parent->isSchema)
            {
                m_records[group->name] = parent->name;
                break;
            }

            parent = !parent->base.empty() ? m_groups[parent->base] : nullptr;
        }
    }

    m_resolved = true;
}

void App::MetadataExporter::WriteInheritanceMap(const std::filesystem::path& aOutPath, bool aGeneratedComment)
{
    ResolveGroups();

    Core::Map<std::string, Core::Set<std::string>> map;

    for (auto& [recordName, schemaName] : m_records)
    {
        auto& record = m_groups[recordName];
        if (record->base != schemaName)
        {
            map[record->base].insert(record->name);
        }
    }

    {
        auto outPath = aOutPath;
        outPath.replace_extension(".yaml");

        std::ofstream out(outPath, std::ios::out);

        if (aGeneratedComment)
        {
            out << "# GENERATED FROM REDMOD SOURCES" << std::endl;
        }

        for (const auto& [groupName, childNames] : map)
        {
            out << groupName << ":" << std::endl;

            for (const auto& childName : childNames)
            {
                out << "- " << childName << std::endl;
            }
        }
    }
}

void App::MetadataExporter::WriteExtraFlats(const std::filesystem::path& aOutPath, bool aGeneratedComment)
{
    ResolveGroups();

    Core::Map<std::string, Core::Map<std::string, Red::TweakFlatPtr>> extras;

    for (auto& [recordName, schemaName] : m_records)
    {
        auto& record = m_groups[recordName];

        for (auto& flat : record->flats)
        {
            if (flat->type == Red::ETweakFlatType::Undefined)
                continue;

            if (extras.contains(schemaName) && extras[schemaName].contains(flat->name))
                continue;

            auto schema = m_groups[schemaName];
            while (schema)
            {
                auto found = std::ranges::any_of(schema->flats, [&flat](auto& aProp) {
                    return aProp->name == flat->name;
                });

                if (found)
                    break;

                schema = !schema->base.empty() ? m_groups[schema->base] : nullptr;
            }

            if (!schema)
            {
                extras[schemaName][flat->name] = flat;
            }
        }
    }

    {
        auto outPath = aOutPath;
        outPath.replace_extension(".yaml");

        std::ofstream out(outPath, std::ios::out);

        if (aGeneratedComment)
        {
            out << "# GENERATED FROM REDMOD SOURCES" << std::endl;
        }

        for (const auto& [schemaName, extraFlats] : extras)
        {
            std::string_view typeName = schemaName;
            typeName.remove_prefix(std::char_traits<char>::length(SchemaPackage) + 1);

            out << typeName << ":" << std::endl;

            for (const auto& [_, flat] : extraFlats)
            {
                out << "  " << flat->name << ":" << std::endl;
                out << "    flatType: " << RedReader::GetFlatTypeName(flat).ToString() << std::endl;

                if (!flat->foreignType.empty())
                {
                    out << "    foreignType: " << flat->foreignType << std::endl;
                }
            }
        }
    }
}
