#include "TweakReader.hpp"
#include "App/Utils/Str.hpp"

namespace
{
constexpr auto PathSeparator = ".";
constexpr auto HashSeparator = "|";

constexpr auto GroupSeparator = ".";
constexpr auto PropSeparator = ".";
constexpr auto InlineSeparator = "$";

constexpr auto IndexOpen = "[";
constexpr auto IndexClose = "]";

constexpr auto ForeignKeyOpen = "<";
constexpr auto ForeignKeyClose = ">";
}

App::BaseTweakReader::BaseTweakReader(Core::SharedPtr<Red::TweakDBManager> aManager)
    : m_manager(std::move(aManager))
    , m_reflection(m_manager->GetReflection())
{
}

bool App::BaseTweakReader::InheritMutations(App::TweakChangeset& aChangeset, Red::TweakDBID aRecordId,
                                          Red::TweakDBID aSourceId)
{
    const auto source = aChangeset.GetRecord(aSourceId);

    if (!source)
        return false;

    const auto recordInfo = m_reflection->GetRecordInfo(source->type);

    if (!recordInfo)
        return false;

    bool inheritedAny = false;

    for (const auto& [_, propInfo] : recordInfo->props)
    {
        if (propInfo->isArray)
        {
            if (aChangeset.InheritChanges(aRecordId + propInfo->appendix, aSourceId + propInfo->appendix))
            {
                inheritedAny = true;
            }
        }
    }

    return inheritedAny;
}

std::string App::BaseTweakReader::ComposeGroupName(const std::string& aParentName, const std::string& aGroupName)
{
    if (aParentName.empty())
        return aGroupName;

    if (aGroupName.empty())
        return aParentName;

    auto groupName = aParentName;
    groupName.append(GroupSeparator);
    groupName.append(aGroupName);

    return groupName;
}

std::string App::BaseTweakReader::ComposeFlatName(const std::string& aParentName, const std::string& aFlatName)
{
    if (aParentName.empty())
        return aFlatName;

    if (aFlatName.empty())
        return aParentName;

    auto flatName = aParentName;
    flatName.append(PropSeparator);
    flatName.append(aFlatName);

    return flatName;
}

std::string App::BaseTweakReader::ComposeInlineName(const std::string& aParentName, const Red::CClass* aRecordType,
                                                    const std::filesystem::path& aSource, int32_t aItemIndex)
{
    auto inlineHash = aSource.string();
    inlineHash.append(HashSeparator);
    inlineHash.append(aParentName);
    inlineHash.append(HashSeparator);
    inlineHash.append(std::to_string(aItemIndex));
    inlineHash.append(HashSeparator);
    inlineHash.append(aRecordType->name.ToString());

    auto inlineName = aParentName;
    inlineName.append(InlineSeparator);
    inlineName.append(ToHex(Red::FNV1a32(inlineHash.c_str(), inlineHash.size())));

    return inlineName;
}

std::string App::BaseTweakReader::ComposePath(const std::string& aParentPath, const std::string& aItemName)
{
    if (aParentPath.empty())
        return aItemName;

    if (aItemName.empty())
        return aParentPath;

    auto itemPath = aParentPath;
    itemPath.append(PathSeparator);
    itemPath.append(aItemName);

    return itemPath;
}

std::string App::BaseTweakReader::ComposePath(const std::string& aParentPath, int32_t aItemIndex)
{
    if (aParentPath.empty())
        return {};

    if (aItemIndex < 0)
        return aParentPath;

    auto itemPath = aParentPath;
    itemPath.append(IndexOpen);
    itemPath.append(std::to_string(aItemIndex));
    itemPath.append(IndexClose);

    return itemPath;
}

const Red::CBaseRTTIType* App::BaseTweakReader::ResolveFlatInstanceType(App::TweakChangeset& aChangeset,
                                                                    Red::TweakDBID aFlatId)
{
    const auto existingFlat = m_manager->GetFlat(aFlatId);
    if (existingFlat.instance)
    {
        return existingFlat.type;
    }

    const auto pendingFlat = aChangeset.GetFlat(aFlatId);
    if (pendingFlat)
    {
        return pendingFlat->type;
    }

    return nullptr;
}

const Red::CClass* App::BaseTweakReader::ResolveRecordInstanceType(App::TweakChangeset& aChangeset,
                                                               Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return nullptr;

    const auto existingRecordType = m_manager->GetRecordType(aRecordId);
    if (existingRecordType)
    {
        return existingRecordType;
    }

    const auto pendingRecord = aChangeset.GetRecord(aRecordId);
    if (pendingRecord)
    {
        return pendingRecord->type;
    }

    return nullptr;
}

std::string App::BaseTweakReader::ToName(const Red::CClass* aType)
{
    return m_reflection->GetRecordShortName(aType->GetName());
}

std::string App::BaseTweakReader::ToName(const Red::CBaseRTTIType* aType, const Red::CClass* aKey)
{
    if (!aType)
        return "<UnknownType>";

    std::string name = aType->GetName().ToString();

    if (aKey)
    {
        name.append(ForeignKeyOpen);
        name.append(m_reflection->GetRecordShortName(aKey->name));
        name.append(ForeignKeyClose);
    }

    return name;
}
