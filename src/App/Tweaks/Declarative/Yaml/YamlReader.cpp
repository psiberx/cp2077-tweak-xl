#include "YamlReader.hpp"
#include "App/Utils/Str.hpp"

namespace
{
constexpr auto AttrSymbol = '$';
constexpr auto TypeAttrKey = "$type";
constexpr auto ValueAttrKey = "$value";
constexpr auto BaseAttrKey = "$base";

constexpr auto PropModeKey = "$props";
constexpr auto PropModeAuto = "AutoFlats";

constexpr auto InlineSeparator = "$";
constexpr auto PropSeparator = ".";
constexpr auto HashSeparator = "|";

constexpr auto AppendOp = RED4ext::FNV1a64("!append");
constexpr auto AppendOnceOp = RED4ext::FNV1a64("!append-once");
constexpr auto AppendFromOp = RED4ext::FNV1a64("!append-from");
constexpr auto PrependOp = RED4ext::FNV1a64("!prepend");
constexpr auto PrependOnceOp = RED4ext::FNV1a64("!prepend-once");
constexpr auto PrependFromOp = RED4ext::FNV1a64("!prepend-from");
constexpr auto MergeOp = RED4ext::FNV1a64("!merge");
constexpr auto RemoveOp = RED4ext::FNV1a64("!remove");

constexpr auto UIIconType = RED4ext::CName("gamedataUIIcon_Record");
constexpr auto StringType = RED4ext::CName("String");

constexpr auto LegacyGroupsNodeKey = "groups";
constexpr auto LegacyMembersNodeKey = "members";
constexpr auto LegacyFlatsNodeKey = "flats";
constexpr auto LegacyTypeNodeKey = "type";
constexpr auto LegacyValueNodeKey = "value";
}

App::YamlReader::YamlReader(Red::TweakDB::Manager& aManager)
    : m_manager(aManager)
    , m_reflection(aManager.GetReflection())
    , m_converter{}
    , m_path{}
    , m_yaml{}
{
}

bool App::YamlReader::Load(const std::filesystem::path& aPath)
{
    m_path = aPath;
    m_yaml = YAML::LoadFile(aPath.string());

    return IsLoaded();
}

bool App::YamlReader::IsLoaded() const
{
    return m_yaml.IsDefined() && !m_yaml.IsNull();
}

void App::YamlReader::Unload()
{
    m_path = "";
    m_yaml = YAML::Node();
}

void App::YamlReader::Read(App::TweakChangeset& aChangeset)
{
    if (!IsLoaded())
        return;

    if (!m_yaml.IsMap())
    {
        LogError("Bad format. Unexpected data at the top level.");
        return;
    }

    ConvertLegacyNodes();

    auto propMode = ResolvePropertyMode(m_yaml);

    for (const auto& it : m_yaml)
    {
        HandleTopNode(aChangeset, propMode, it.first.Scalar(), it.second);
    }
}

App::YamlReader::PropertyMode App::YamlReader::ResolvePropertyMode(const YAML::Node& aNode, PropertyMode aDefault)
{
    const auto modeAttr = aNode[PropModeKey];

    if (modeAttr.IsDefined() && modeAttr.Scalar() == PropModeAuto)
    {
        return PropertyMode::Auto;
    }

    return aDefault;
}

void App::YamlReader::HandleTopNode(App::TweakChangeset& aChangeset, PropertyMode aPropMode,
                                    const std::string& aName, const YAML::Node& aNode)
{
    if (aName.empty())
    {
        LogError("Bad format. The top element must have a name.");
        return;
    }

    const auto targetId = RED4ext::TweakDBID(aName);

    switch (aNode.Type())
    {
    case YAML::NodeType::Map:
    {
        {
            const auto recordType = ResolveRecordType(aChangeset, targetId);

            if (recordType)
            {
                auto cloneAttr = aNode[BaseAttrKey];

                if (cloneAttr.IsDefined())
                {
                    const auto sourceId = ResolveTweakDBID(cloneAttr);
                    const auto sourceType = ResolveRecordType(aChangeset, sourceId);

                    if (!sourceType)
                    {
                        LogWarning("{}: Cannot clone [{}], the record doesn't exists.", aName, cloneAttr.Scalar());
                        break;
                    }

                    if (sourceType != recordType)
                    {
                        LogWarning("{}: Cannot clone [{}], the record has incompatible type.", aName, cloneAttr.Scalar());
                        break;
                    }

                    HandleRecordNode(aChangeset, aPropMode, aName, aName, aNode, recordType, sourceId);
                }
                else
                {
                    HandleRecordNode(aChangeset, aPropMode, aName, aName, aNode, recordType);
                }
                break;
            }
        }

        {
            const auto flatType = ResolveFlatType(aChangeset, targetId);

            if (flatType)
            {
                HandleFlatNode(aChangeset, aName, aNode, flatType);
                break;
            }
        }

        {
            auto cloneAttr = aNode[BaseAttrKey];

            if (cloneAttr.IsDefined())
            {
                const auto sourceId = ResolveTweakDBID(cloneAttr);
                const auto sourceType = ResolveRecordType(aChangeset, sourceId);

                if (!sourceType)
                {
                    LogWarning("{}: Cannot clone [{}], the record doesn't exists.", aName, cloneAttr.Scalar());
                    break;
                }

                HandleRecordNode(aChangeset, aPropMode, aName, aName, aNode, sourceType, sourceId);
                break;
            }
        }

        {
            const auto typeAttr = aNode[TypeAttrKey];

            if (typeAttr.IsDefined())
            {
                const auto valueAttr = aNode[ValueAttrKey];

                if (valueAttr.IsDefined())
                {
                    const auto flatType = ResolveFlatType(typeAttr);

                    if (!flatType)
                    {
                        LogWarning("{}: Invalid value type [{}].", aName, typeAttr.Scalar());
                        break;
                    }

                    HandleFlatNode(aChangeset, aName, valueAttr, flatType);
                    break;
                }
                else
                {
                    const auto recordType = ResolveRecordType(typeAttr);

                    if (!recordType)
                    {
                        LogWarning("{}: Invalid record type [{}].", aName, typeAttr.Scalar());
                        break;
                    }

                    HandleRecordNode(aChangeset, aPropMode, aName, aName, aNode, recordType);
                    break;
                }
            }
        }

        // Try to infer the flat based on the content
        HandleFlatNode(aChangeset, aName, aNode);
        break;
    }
    case YAML::NodeType::Scalar:
    case YAML::NodeType::Sequence:
    {
        const auto flat = m_manager.GetFlat(targetId);

        if (!flat.value)
        {
            HandleFlatNode(aChangeset, aName, aNode);
            break;
        }

        HandleFlatNode(aChangeset, aName, aNode, flat.type);
        break;
    }
    default:
    {
        LogError("{}: Bad format. Unexpected data.", aName);
        break;
    }
    }
}

void App::YamlReader::HandleFlatNode(App::TweakChangeset& aChangeset, const std::string& aName, const YAML::Node& aNode,
                                     const RED4ext::CBaseRTTIType* aType)
{
    const RED4ext::CBaseRTTIType* flatType;
    Core::SharedPtr<void> flatValue;

    if (aType != nullptr)
    {
        if (Red::TweakDB::IsArrayType(aType))
        {
            const auto elementType = ResolveFlatType(Red::TweakDB::GetElementType(aType));

            if (HandleRelativeChanges(aChangeset, aName, aName, aNode, elementType))
                return;
        }

        flatType = aType;
        flatValue = m_converter.Convert(aType, aNode);

        if (!flatValue)
        {
            LogError("{}: Invalid value. Expected {}.", aName, aType->GetName().ToString());
            return;
        }
    }
    else
    {
        const auto x = m_converter.Convert(aNode);

        if (!x.second)
        {
            LogError("{}: Ambiguous definition. The value type cannot be determined.", aName);
            return;
        }

        flatType = m_reflection.GetFlatType(x.first);
        flatValue = x.second;
    }

    const auto flatId = RED4ext::TweakDBID(aName);

    aChangeset.SetFlat(flatId, flatType, flatValue);
    aChangeset.RegisterName(flatId, aName);

    {
        const auto separatorPos = aName.find_last_of(PropSeparator);

        if (separatorPos != std::string::npos)
        {
            const auto recordName = aName.substr(0, separatorPos);
            const auto recordId = Red::TweakDBID(recordName);

            if (m_manager.IsRecordExists(recordId))
            {
                aChangeset.UpdateRecord(recordId);
            }

            aChangeset.AssociateRecord(recordId, flatId);
        }
    }
}

void App::YamlReader::HandleRecordNode(App::TweakChangeset& aChangeset, PropertyMode aPropMode,
                                       const std::string& aPath, const std::string& aName,
                                       const YAML::Node& aNode, const RED4ext::CClass* aType,
                                       RED4ext::TweakDBID aSourceId)
{
    const auto recordId = RED4ext::TweakDBID(aName);
    const auto recordInfo = m_reflection.GetRecordInfo(aType);

    if (!recordInfo)
    {
        if (Red::TweakDB::IsRecordType(aType))
            LogError("{}: Cannot create record, the record type [{}] is abstract.", aPath, aType->name.ToString());
        else
            LogError("{}: Cannot create record, the record type [{}] is unknown.", aPath, aType->name.ToString());
        return;
    }

    aChangeset.MakeRecord(recordId, aType, aSourceId);

    if (aSourceId.IsValid() && aChangeset.HasRecord(aSourceId))
    {
        for (const auto& [_, propInfo] : recordInfo->props)
        {
            if (propInfo->isArray)
            {
                const auto propId = RED4ext::TweakDBID(recordId, propInfo->appendix);
                const auto baseId = RED4ext::TweakDBID(aSourceId, propInfo->appendix);

                if (aChangeset.InheritChanges(propId, baseId))
                {
                    aChangeset.AssociateRecord(recordId, propId);
                }
            }
        }
    }

    const auto propMode = ResolvePropertyMode(aNode, aPropMode);

    for (const auto& nodeIt : aNode)
    {
        const auto nodeKey = nodeIt.first.Scalar();

        // Skip attributes
        if (nodeKey[0] == AttrSymbol)
            continue;

        const auto propKey = RED4ext::CName(nodeKey.c_str());
        const auto propInfo = recordInfo->GetPropInfo(propKey);

        if (!propInfo)
        {
            if (propMode == PropertyMode::Auto)
            {
                auto propName = aName;
                propName.append(PropSeparator);
                propName.append(nodeKey);

                HandleFlatNode(aChangeset, propName, nodeIt.second);
            }
            else
            {
                LogError("{}: Unknown property {}.{}.", aPath, recordInfo->shortName, nodeKey);
            }
            continue;
        }

        const auto propId = RED4ext::TweakDBID(recordId, propInfo->appendix);

        aChangeset.AssociateRecord(recordId, propId);

        const auto originalData = nodeIt.second;
        YAML::Node overrideData;

        // Inline records
        if (propInfo->isForeignKey)
        {
            if (propInfo->isArray && originalData.IsSequence())
            {
                for (std::size_t itemIndex = 0; itemIndex < originalData.size(); ++itemIndex)
                {
                    auto itemData = originalData[itemIndex];

                    if (itemData.IsMap())
                    {
                        auto inlinePath = aPath;
                        inlinePath.append(propInfo->appendix);
                        inlinePath.append(PropSeparator);
                        inlinePath.append(std::to_string(itemIndex));

                        auto sourceId = RED4ext::TweakDBID();
                        auto foreignType = propInfo->foreignType;

                        if (!ResolveInlineNode(aChangeset, inlinePath, itemData, foreignType, sourceId))
                            continue;

                        auto inlineHash = m_path.string();
                        inlineHash.append(HashSeparator);
                        inlineHash.append(aName);
                        inlineHash.append(HashSeparator);
                        inlineHash.append(nodeKey);
                        inlineHash.append(HashSeparator);
                        inlineHash.append(std::to_string(itemIndex));
                        inlineHash.append(HashSeparator);
                        inlineHash.append(foreignType->name.ToString());

                        auto inlineName = aName;
                        inlineName.append(InlineSeparator);
                        inlineName.append(nodeKey);
                        inlineName.append(InlineSeparator);
                        inlineName.append(ToHex(Red::FNV1a32(inlineHash.c_str(), inlineHash.size())));

                        HandleRecordNode(aChangeset, propMode, inlinePath, inlineName, itemData, foreignType, sourceId);

                        if (overrideData.IsNull())
                        {
                            overrideData = YAML::Clone(originalData);
                        }

                        // Overwrite inline item with foreign key
                        overrideData[itemIndex] = inlineName;
                    }
                }
            }
            else if (originalData.IsMap())
            {
                auto inlinePath = aPath;
                inlinePath.append(propInfo->appendix);

                auto sourceId = RED4ext::TweakDBID();
                auto foreignType = propInfo->foreignType;

                if (!ResolveInlineNode(aChangeset, inlinePath, originalData, foreignType, sourceId))
                    continue;

                auto inlineName = aName;
                inlineName.append(InlineSeparator);
                inlineName.append(nodeKey);

                // Special handling for UIIcon
                if (propInfo->foreignType->GetName() == UIIconType)
                {
                    // Item records have both .iconPath and .icon properties, but last one is never used.
                    // So if parent record has .iconPath property then auto fill it with our inline icon name.
                    if (recordInfo->props.contains("iconPath") && !aNode["iconPath"])
                    {
                        aChangeset.SetFlat(RED4ext::TweakDBID(recordId, ".iconPath"), ResolveFlatType("String"),
                                       Core::MakeShared<RED4ext::CString>(inlineName.c_str()));
                    }

                    // Then force type prefix to make it accessible by short name that we just set in .iconPath.
                    inlineName.insert(0, "UIIcon.");
                }

                HandleRecordNode(aChangeset, propMode, inlinePath, inlineName, originalData, foreignType, sourceId);

                // Overwrite inline data with foreign key
                overrideData = inlineName;
            }
        }

        const auto& nodeData = !overrideData.IsNull() ? overrideData : originalData;

        // Relative operations
        if (propInfo->isArray)
        {
            auto propName = aName;
            propName.append(propInfo->appendix);

            auto propPath = aPath;
            propPath.append(propInfo->appendix);

            if (HandleRelativeChanges(aChangeset, propPath, propName, nodeData, propInfo->elementType))
                continue;
        }

        const auto propValue = m_converter.Convert(propInfo->type, nodeData);

        if (!propValue)
        {
            LogError("{}.{}: Invalid value, expected {}.", aPath, nodeKey, propInfo->type->GetName().ToString());
            continue;
        }

        aChangeset.SetFlat(propId, propInfo->type, propValue);
        aChangeset.RegisterName(propId, aName + propInfo->appendix);
    }

    aChangeset.RegisterName(recordId, aName);
}

bool App::YamlReader::ResolveInlineNode(App::TweakChangeset& aChangeset, const std::string& aPath,
                                        const YAML::Node& aNode, const RED4ext::CClass*& aForeignType,
                                        RED4ext::TweakDBID& aSourceId)
{
    {
        const auto cloneAttr = aNode[BaseAttrKey];

        if (cloneAttr.IsDefined())
        {
            const auto sourceId = ResolveTweakDBID(cloneAttr);
            const auto sourceType = ResolveRecordType(aChangeset, sourceId);

            if (sourceType)
            {
                if (sourceType->IsA(aForeignType))
                {
                    aSourceId = sourceId;
                    aForeignType = sourceType;
                    return true;
                }
                else
                {
                    LogError("{}: Cannot inline from [{}], the record has incompatible type.",
                             aPath, cloneAttr.Scalar());
                }
            }
            else
            {
                LogWarning("{}: Cannot clone from [{}], the record doesn't exists.",
                           aPath, cloneAttr.Scalar());
            }

            return false;
        }
    }

    {
        const auto typeAttr = aNode[TypeAttrKey];

        if (typeAttr.IsDefined())
        {
            const auto suggestedType = ResolveRecordType(typeAttr);

            if (suggestedType)
            {
                if (suggestedType->IsA(aForeignType))
                {
                    aForeignType = suggestedType;
                    return true;
                }
                else
                {
                    LogError("{}: Cannot be inlined, provided type [{}] is incompatible with property type.",
                             aPath, typeAttr.Scalar());
                }
            }
            else
            {
                LogError("{}: Cannot be inlined, provided type [{}] is not a known record type or abstract.",
                         aPath, typeAttr.Scalar());

            }

            return false;
        }
    }

    return true;
}

bool App::YamlReader::HandleRelativeChanges(TweakChangeset& aChangeset, const std::string& aPath,
                                            const std::string& aName, const YAML::Node& aNode,
                                            const RED4ext::CBaseRTTIType* aElementType)
{
    if (!aNode.IsSequence())
        return false;

    const auto flatId = RED4ext::TweakDBID(aName);

    bool isRelative = false;
    bool isAbsolute = false;

    for (std::size_t itemIndex = 0; itemIndex < aNode.size(); ++itemIndex)
    {
        auto itemData = aNode[itemIndex];

        if (itemData.Tag().length() <= 1)
        {
            isAbsolute = true;
            continue;
        }

        const auto tag = RED4ext::FNV1a64(itemData.Tag().c_str());

        switch (tag)
        {
        case AppendOp:
        case AppendOnceOp:
        {
            const auto itemValue = m_converter.Convert(aElementType, itemData);

            if (!itemValue)
            {
                LogError("{}.{}: Invalid value, expected {}.",
                         aPath, std::to_string(itemIndex), aElementType->GetName().ToString());
                continue;
            }

            aChangeset.AppendElement(flatId, aElementType, itemValue, tag == AppendOnceOp);
            isRelative = true;
            break;
        }
        case PrependOp:
        case PrependOnceOp:
        {
            const auto itemValue = m_converter.Convert(aElementType, itemData);

            if (!itemValue)
            {
                LogError("{}.{}: Invalid value, expected {}.",
                         aPath, std::to_string(itemIndex), aElementType->GetName().ToString());
                continue;
            }

            aChangeset.PrependElement(flatId, aElementType, itemValue, tag == PrependOnceOp);
            isRelative = true;
            break;
        }
        case MergeOp:
        case AppendFromOp:
        {
            const auto sourceId = ResolveTweakDBID(itemData);

            if (!sourceId.IsValid())
            {
                LogError("{}.{}: Invalid value, expected [TweakDBID].", aPath, std::to_string(itemIndex));
                continue;
            }

            aChangeset.AppendFrom(flatId, sourceId);
            isRelative = true;
            break;
        }
        case PrependFromOp:
        {
            const auto sourceId = ResolveTweakDBID(itemData);

            if (!sourceId.IsValid())
            {
                LogError("{}.{}: Invalid value, expected [TweakDBID].", aPath, std::to_string(itemIndex));
                continue;
            }

            aChangeset.PrependFrom(flatId, sourceId);
            isRelative = true;
            break;
        }
        case RemoveOp:
        {
            const auto itemValue = m_converter.Convert(aElementType, itemData);

            if (!itemValue)
            {
                LogError("{}.{}: Invalid value, expected {}.",
                         aPath, std::to_string(itemIndex), aElementType->GetName().ToString());
                continue;
            }

            aChangeset.RemoveElement(flatId, aElementType, itemValue);
            isRelative = true;
            break;
        }
        default:
        {
            LogError("{}.{}: Invalid action [{}].", aPath, std::to_string(itemIndex), itemData.Tag());
        }
        }
    }

    if (isRelative && isAbsolute)
    {
        LogWarning("{}: Mixed definition with array replacement and relative changes. "
                   "Only relative changes will take effect.", aPath);
    }

    return isRelative;
}

bool App::YamlReader::IsRelativeChange(const YAML::Node& aNode)
{
    if (aNode.Tag().length() <= 1)
        return false;

    const auto tag = RED4ext::FNV1a64(aNode.Tag().c_str());

    switch (tag)
    {
    case AppendOp:
    case AppendOnceOp:
    case AppendFromOp:
    case PrependOp:
    case PrependOnceOp:
    case PrependFromOp:
    case MergeOp:
    case RemoveOp:
        return true;
    default:
        return false;
    }
}

const RED4ext::CBaseRTTIType* App::YamlReader::ResolveFlatType(const YAML::Node& aNode)
{
    return m_reflection.GetFlatType(aNode.Scalar().c_str());
}

const RED4ext::CBaseRTTIType* App::YamlReader::ResolveFlatType(RED4ext::CName aName)
{
    return m_reflection.GetFlatType(aName);
}

const RED4ext::CBaseRTTIType* App::YamlReader::ResolveFlatType(App::TweakChangeset& aChangeset,
                                                               RED4ext::TweakDBID aFlatId)
{
    const auto existingFlat = m_manager.GetFlat(aFlatId);
    if (existingFlat.value)
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

const RED4ext::CClass* App::YamlReader::ResolveRecordType(const YAML::Node& aNode)
{
    return m_reflection.GetRecordType(aNode.Scalar().c_str());
}

const RED4ext::CClass* App::YamlReader::ResolveRecordType(App::TweakChangeset& aChangeset, RED4ext::TweakDBID aRecordId)
{
    const auto existingRecord = m_manager.GetRecord(aRecordId);
    if (existingRecord)
    {
        return existingRecord->GetType();
    }

    const auto pendingRecord = aChangeset.GetRecord(aRecordId);
    if (pendingRecord)
    {
        return pendingRecord->type;
    }

    return nullptr;
}

RED4ext::TweakDBID App::YamlReader::ResolveTweakDBID(const YAML::Node& aNode)
{
    if (!aNode.IsDefined())
        return {};

    const auto resolvedId = m_converter.Convert<RED4ext::TweakDBID>(aNode);

    if (!resolvedId)
        return {};

    return *resolvedId;
}

void App::YamlReader::ConvertLegacyNodes()
{
    const auto groupsNode = m_yaml[LegacyGroupsNodeKey];

    if (groupsNode.IsMap())
    {
        for (const auto& groupIt : groupsNode)
        {
            const auto groupKey = groupIt.first;
            const auto groupNode = groupIt.second;

            if (!groupKey.IsDefined() || !groupNode.IsMap())
                continue;

            const auto groupTypeNode = groupNode[LegacyTypeNodeKey];
            const auto groupMembersNode = groupNode[LegacyMembersNodeKey];

            if (!groupTypeNode.IsDefined() || !groupMembersNode.IsMap())
                continue;

            auto convertedNode = YAML::Node();
            convertedNode[TypeAttrKey] = groupTypeNode;

            for (const auto& memberIt : groupMembersNode)
            {
                const auto memberKey = memberIt.first;
                const auto memberNode = memberIt.second;

                if (!memberKey.IsDefined() || !memberNode.IsMap())
                    continue;

                const auto memberTypeNode = groupNode[LegacyTypeNodeKey];
                const auto memberValueNode = groupNode[LegacyValueNodeKey];

                if (!memberTypeNode.IsDefined() || !memberValueNode.IsDefined())
                    continue;

                convertedNode[memberKey] = memberValueNode;
            }

            m_yaml[groupKey] = convertedNode;
        }

        m_yaml.remove(LegacyGroupsNodeKey);
    }

    const auto flatsNode = m_yaml[LegacyFlatsNodeKey];

    if (flatsNode.IsMap())
    {
        for (const auto& flatIt : flatsNode)
        {
            const auto flatKey = flatIt.first;
            const auto flatNode = flatIt.second;

            if (!flatKey.IsDefined() || !flatNode.IsMap())
                continue;

            const auto flatTypeNode = flatNode[LegacyTypeNodeKey];
            const auto flatValueNode = flatNode[LegacyValueNodeKey];

            if (!flatTypeNode.IsDefined() || !flatValueNode.IsDefined())
                continue;

            auto convertedNode = YAML::Node();
            convertedNode[TypeAttrKey] = flatTypeNode;
            convertedNode[ValueAttrKey] = flatValueNode;

            m_yaml[flatKey] = convertedNode;
        }

        m_yaml.remove(LegacyFlatsNodeKey);
    }
}
