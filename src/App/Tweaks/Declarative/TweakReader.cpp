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
} // namespace

App::BaseTweakReader::BaseTweakReader(const Core::DeferredPtr<Red::TweakDBManager>& aManager,
                                      const Core::DeferredPtr<Red::TweakDBReflection>& aReflection,
                                      const Core::SharedPtr<ScriptableRecordManager>& aRecordManager,
                                      const Core::SharedPtr<TweakContext>& aContext)
    : m_manager(aManager)
    , m_reflection(aReflection)
    , m_recordManager(aRecordManager)
    , m_context(aContext)
{
}

bool App::BaseTweakReader::IsOriginalBaseRecord(const Red::TweakDBID aRecordId)
{
    if (!m_reflection && m_manager)
    {
        m_reflection = m_manager->GetReflection();
    }

    if (!m_reflection)
    {
        return false;
    }

    return m_reflection->IsOriginalBaseRecord(aRecordId);
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
                                                    const std::filesystem::path& aSource, const int32_t aItemIndex)
{
    auto inlineHash = aSource.string();
    inlineHash.append(HashSeparator);
    inlineHash.append(aParentName);
    inlineHash.append(HashSeparator);
    inlineHash.append(aRecordType->name.ToString());

    if (aItemIndex >= 0)
    {
        inlineHash.append(HashSeparator);
        inlineHash.append(std::to_string(aItemIndex));
        inlineHash.append(HashSeparator);
        inlineHash.append(std::to_string(++m_inlineIndexSuffix[inlineHash]));
    }

    auto inlineName = aParentName;
    inlineName.append(InlineSeparator);
    inlineName.append(ToHex(Red::FNV1a32(inlineHash.data(), inlineHash.size())));

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

std::string App::BaseTweakReader::ComposePath(const std::string& aParentPath, const int32_t aItemIndex)
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

const Red::CBaseRTTIType* App::BaseTweakReader::ResolveFlatInstanceType(TweakChangeset& aChangeset,
                                                                        const Red::TweakDBID aFlatId)
{
    if (const auto existingFlat = m_manager->GetFlat(aFlatId))
    {
        return existingFlat.type;
    }

    if (const auto pendingFlat = aChangeset.GetFlat(aFlatId))
    {
        return pendingFlat->type;
    }

    return nullptr;
}

const Red::CClass* App::BaseTweakReader::ResolveRecordInstanceType(TweakChangeset& aChangeset,
                                                                   const Red::TweakDBID aRecordId)
{
    if (!aRecordId.IsValid())
        return nullptr;

    if (const auto existingRecordType = m_manager->GetRecordType(aRecordId))
    {
        return existingRecordType;
    }

    if (const auto pendingRecord = aChangeset.GetRecord(aRecordId))
    {
        return pendingRecord->type;
    }

    return nullptr;
}

std::string App::BaseTweakReader::ToName(const Red::CClass* aType)
{
    return Red::GetRecordShortName<std::string>(aType->GetName());
}

std::string App::BaseTweakReader::ToName(const Red::CBaseRTTIType* aType, const Red::CClass* aKey)
{
    if (!aType)
        return "<UnknownType>";

    std::string name = aType->GetName().ToString();

    if (aKey)
    {
        name.append(ForeignKeyOpen);
        name.append(Red::GetRecordShortName<std::string>(aKey->name));
        name.append(ForeignKeyClose);
    }

    return name;
}
