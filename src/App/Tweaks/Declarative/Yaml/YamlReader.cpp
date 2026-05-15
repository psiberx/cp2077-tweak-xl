#include "YamlReader.hpp"

namespace
{
constexpr auto AttrSymbol = '$';
constexpr auto TypeAttrKey = "$type";
constexpr auto ValueAttrKey = "$value";
constexpr auto BaseAttrKey = "$base";
constexpr auto PropModeKey = "$props";
constexpr auto PropModeAuto = "AutoFlats";
constexpr auto GameConditionKey = "$game";
constexpr auto DLCConditionKey = "$dlc";
constexpr auto SchemaTypeValue = "Schema";

constexpr auto AppendOp = Red::FNV1a64("!append");
constexpr auto AppendOnceOp = Red::FNV1a64("!append-once");
constexpr auto AppendFromOp = Red::FNV1a64("!append-from");
constexpr auto PrependOp = Red::FNV1a64("!prepend");
constexpr auto PrependOnceOp = Red::FNV1a64("!prepend-once");
constexpr auto PrependFromOp = Red::FNV1a64("!prepend-from");
constexpr auto MergeOp = Red::FNV1a64("!merge");
constexpr auto RemoveOp = Red::FNV1a64("!remove");
constexpr auto RemoveAllOp = Red::FNV1a64("!remove-all");

constexpr auto UIIconType = Red::CName("gamedataUIIcon_Record");

constexpr auto PropSeparator = ".";
} // namespace

App::YamlReader::YamlReader(const Core::DeferredPtr<Red::TweakDBManager>& aManager,
                            const Core::DeferredPtr<Red::TweakDBReflection>& aReflection,
                            const Core::SharedPtr<ScriptableRecordManager>& aRecordManager,
                            const Core::SharedPtr<TweakContext>& aContext)

    : BaseTweakReader(aManager, aReflection, aRecordManager, aContext)
{
}

bool App::YamlReader::Load(const std::filesystem::path& aPath)
{
    m_path = aPath;
    m_data = YAML::LoadFile(aPath.string());

    if (!m_data.IsDefined() || m_data.IsNull())
        return false;

    if (!m_data.IsMap())
    {
        LogError("Bad format in path {}: Unexpected data at the top level.", m_path.string());
        return false;
    }

    if (!CheckConditions(m_data))
        return false;

    ProcessTemplates(m_data);

    m_isLoaded = true;
    return true;
}

bool App::YamlReader::IsLoaded() const
{
    return m_data.IsDefined() && !m_data.IsNull();
}

void App::YamlReader::Unload()
{
    m_path = "";
    m_data = YAML::Node();
}

void App::YamlReader::ReadSchemas(SchemaChangeset& aChangeset)
{
    if (!IsLoaded())
        return;

    for (const auto& it : m_data)
        HandleSchemaNode(aChangeset, it.first.Scalar(), it.second);
}

void App::YamlReader::ReadValues(TweakChangeset& aChangeset)
{
    if (!IsLoaded())
        return;

    if (!m_reflection || !m_manager)
        return;

    const auto propMode = ResolvePropertyMode(m_data);

    for (const auto& it : m_data)
        HandleTopNode(aChangeset, propMode, it.first.Scalar(), it.second);
}

void App::YamlReader::HandleSchemaNode(SchemaChangeset& aChangeset, const std::string& aRecordName,
                                       const YAML::Node& aNode)
{
    if (aRecordName.empty())
    {
        LogError("Bad format in path {}: The top element must have a name.", m_path.string());
        return;
    }

    if (aRecordName[0] == AttrSymbol)
        return;

    if (aNode.Type() != YAML::NodeType::Map)
        return;

    if (!CheckConditions(aNode))
        return;

    const auto typeAttr = aNode[TypeAttrKey];

    if (!typeAttr.IsDefined())
        return;

    if (!typeAttr.IsScalar())
    {
        LogError("{}: Invalid $type value.", aRecordName);
        return;
    }

    if (typeAttr.Scalar() != SchemaTypeValue)
        return;

    std::optional<std::string> parent = std::nullopt;

    if (const auto baseAttr = aNode[BaseAttrKey]; baseAttr.IsDefined())
    {
        if (!baseAttr.IsScalar())
        {
            LogError("{}: Invalid base record name.", aRecordName);
            return;
        }

        if (const auto& val = baseAttr.Scalar(); !val.empty())
        {
            parent = val;
        }
    }

    if (!aChangeset.MakeRecord(aRecordName, parent))
        return;

    for (const auto& nodeIt : aNode)
        HandleSchemaPropertyNode(aChangeset, aRecordName, nodeIt.first.Scalar(), nodeIt.second);
}

void App::YamlReader::HandleSchemaPropertyNode(SchemaChangeset& aChangeset, const std::string& aRecordName,
                                               const std::string& aPropName, const YAML::Node& aNode)
{
    if (aPropName.empty() || aPropName[0] == AttrSymbol)
        return;

    const auto recordName = Red::GetRecordFullName<std::string>(aRecordName);

    if (aNode.IsMap())
    {
        const auto typeAttr = aNode[TypeAttrKey];
        const auto valueAttr = aNode[ValueAttrKey];

        if (typeAttr.IsDefined() && valueAttr.IsDefined())
        {
            const auto typeSpec = ResolvePropertyFlatInfo(typeAttr);

            if (!typeSpec)
            {
                LogError("{}: Invalid type {} for property type {}.", aRecordName, typeAttr.Scalar(), aPropName);
                return;
            }

            const auto [propType, propInstance] = TryMakeValue(valueAttr);

            if (propInstance && propType != typeSpec->flatTypeName)
            {
                LogError("{}: Invalid value type for property {}. Expected {}, got {}.", aRecordName, aPropName,
                         typeSpec->flatTypeName.ToString(), propType.ToString());
                return;
            }

            aChangeset.MakeProperty(aRecordName, aPropName, typeSpec, propInstance);
            return;
        }
    }

    if (const auto& [propType, propInstance] = TryMakeValue(aNode); propInstance)
    {
        if (Red::IsForeignKey(propType) || Red::IsForeignKeyArray(propType))
        {
            LogError("{}: Invalid type for property {}. Foreign keys must be defined as a map containing '$type' and "
                     "'$value' keys.",
                     aRecordName, aPropName);
            return;
        }

        const auto typeSpec = GetTweakTypeSpec(propType.ToString());

        if (!typeSpec)
        {
            LogError("{}: Invalid type {} for property type {}.", aRecordName, propType.ToString(), aPropName);
            return;
        }

        aChangeset.MakeProperty(aRecordName, aPropName, typeSpec, propInstance);
        return;
    }

    LogError("{}: Unable to infer type for property {}.", aRecordName, aPropName);
}

void App::YamlReader::HandleTopNode(TweakChangeset& aChangeset, PropertyMode aPropMode, const std::string& aName,
                                    const YAML::Node& aNode)
{
    if (aName.empty())
    {
        LogError("Bad format. The top element must have a name.");
        return;
    }

    // Skip attributes
    if (aName[0] == AttrSymbol)
        return;

    const auto targetId = Red::TweakDBID(aName);

    switch (aNode.Type())
    {
    case YAML::NodeType::Map:
    {
        if (const auto recordType = ResolveRecordInstanceType(aChangeset, targetId))
        {
            if (auto cloneAttr = aNode[BaseAttrKey]; cloneAttr.IsDefined())
            {
                const auto sourceId = ResolveTweakDBID(cloneAttr);
                const auto sourceType = ResolveRecordInstanceType(aChangeset, sourceId);

                if (!sourceType)
                {
                    LogError("{}: Cannot clone {}, the record doesn't exist.", aName, cloneAttr.Scalar());
                    break;
                }

                if (sourceType != recordType)
                {
                    LogError("{}: Cannot clone {}, the record has incompatible type.", aName, cloneAttr.Scalar());
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

        if (const auto flatType = ResolveFlatInstanceType(aChangeset, targetId))
        {
            const auto& valueAttr = aNode[ValueAttrKey];
            HandleFlatNode(aChangeset, aName, valueAttr.IsDefined() ? valueAttr : aNode, flatType);
            break;
        }

        if (auto cloneAttr = aNode[BaseAttrKey]; cloneAttr.IsDefined())
        {
            const auto sourceId = ResolveTweakDBID(cloneAttr);
            const auto sourceType = ResolveRecordInstanceType(aChangeset, sourceId);

            if (!sourceType)
            {
                LogError("{}: Cannot clone {}, the record doesn't exist.", aName, cloneAttr.Scalar());
                break;
            }

            HandleRecordNode(aChangeset, aPropMode, aName, aName, aNode, sourceType, sourceId);
            break;
        }

        if (const auto typeAttr = aNode[TypeAttrKey]; typeAttr.IsDefined())
        {
            if (typeAttr.IsScalar() && typeAttr.Scalar() == SchemaTypeValue)
            {
                break;
            }

            if (const auto valueAttr = aNode[ValueAttrKey]; valueAttr.IsDefined())
            {
                const auto flatType = ResolveFlatType(typeAttr);

                if (!flatType)
                {
                    LogWarning("{}: Invalid value type {}.", aName, typeAttr.Scalar());
                    break;
                }

                HandleFlatNode(aChangeset, aName, valueAttr, flatType);
                break;
            }

            const auto recordType = ResolveRecordType(typeAttr);

            if (!recordType)
            {
                LogWarning("{}: Invalid record type {}.", aName, typeAttr.Scalar());
                break;
            }

            HandleRecordNode(aChangeset, aPropMode, aName, aName, aNode, recordType);
            break;
        }

        // Try to infer the flat based on the content
        HandleFlatNode(aChangeset, aName, aNode);
        break;
    }
    case YAML::NodeType::Scalar:
    case YAML::NodeType::Sequence:
    {
        HandleFlatNode(aChangeset, aName, aNode, ResolveFlatInstanceType(aChangeset, targetId));
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
                                     const Red::CBaseRTTIType* aType)
{
    const auto flatId = Red::TweakDBID(aName);
    const Red::CBaseRTTIType* flatType;
    Red::InstancePtr<> flatValue;

    aChangeset.RegisterName(flatId, aName);

    if (aType != nullptr)
    {
        flatType = aType;

        if (Red::IsArrayType(flatType))
        {
            const auto elementType = ResolveFlatType(Red::GetElementTypeName(flatType));

            if (HandleMutations(aChangeset, aName, aName, aNode, elementType))
            {
                UpdateFlatOwner(aChangeset, aName);
                return;
            }
        }

        flatValue = MakeValue(aType, aNode);

        if (!flatValue)
        {
            LogError("{}: Invalid value. Expected {}.", aName, aType->GetName().ToString());
            return;
        }
    }
    else
    {
        const auto x = TryMakeValue(aNode);

        if (!x.second)
        {
            LogError("{}: Ambiguous definition. The value type cannot be determined.", aName);
            return;
        }

        flatType = Red::GetFlatType(x.first);

        if (Red::IsArrayType(flatType))
        {
            const auto elementType = ResolveFlatType(Red::GetElementTypeName(flatType));

            if (HandleMutations(aChangeset, aName, aName, aNode, elementType))
            {
                UpdateFlatOwner(aChangeset, aName);
                return;
            }
        }

        flatValue = x.second;
    }

    aChangeset.SetFlat(flatId, flatType, flatValue);

    UpdateFlatOwner(aChangeset, aName);
}

void App::YamlReader::HandleRecordNode(TweakChangeset& aChangeset, PropertyMode aPropMode,
                                       const std::string& aRecordPath, const std::string& aRecordName,
                                       const YAML::Node& aNode, const Red::CClass* aRecordType,
                                       Red::TweakDBID aSourceId)
{
    const auto recordId = Red::TweakDBID(aRecordName);
    const auto recordInfo = m_reflection->GetRecordInfo(aRecordType);

    if (!recordInfo)
    {
        if (Red::IsRecordType(aRecordType))
            LogError("{}: Cannot create record, the record type {} is abstract.", aRecordPath,
                     Red::GetRecordShortName<std::string>(aRecordType->GetName()));
        else
            LogError("{}: Cannot create record, {} is not a record type.", aRecordPath,
                     aRecordType->GetName().ToString());
        return;
    }

    if (recordId == aSourceId)
    {
        LogError("{}: Cannot clone {} from itself.", aRecordPath, aRecordName);
        return;
    }

    if (!CheckConditions(aNode))
        return;

    aChangeset.MakeRecord(recordId, aRecordType, aSourceId);
    aChangeset.RegisterName(recordId, aRecordName);

    const auto propMode = ResolvePropertyMode(aNode, aPropMode);
    const auto isOriginalBase = IsOriginalBaseRecord(recordId);

    for (const auto& nodeIt : aNode)
    {
        const auto nodeKey = nodeIt.first.Scalar();

        // Skip attributes
        if (nodeKey[0] == AttrSymbol)
            continue;

        const auto propName = ComposeFlatName(aRecordName, nodeKey);
        const auto propPath = ComposeFlatName(aRecordPath, nodeKey);

        const auto propInfo = recordInfo->GetPropInfo(nodeKey.c_str());

        if (!propInfo)
        {
            if (propMode == PropertyMode::Auto)
            {
                HandleFlatNode(aChangeset, propName, nodeIt.second);
            }
            else
            {
                LogError("{}: Unknown property {}.", aRecordPath, nodeKey);
            }
            continue;
        }

        const auto propId = Red::TweakDBID(propName);
        const auto originalData = nodeIt.second;
        YAML::Node overrideData;

        // Inline records
        if (propInfo->isForeignKey)
        {
            if (propInfo->isArray && originalData.IsSequence())
            {
                auto inlineFailed = false;

                for (auto itemIndex = 0; itemIndex < originalData.size(); ++itemIndex)
                {
                    if (auto itemData = originalData[itemIndex]; itemData.IsMap())
                    {
                        auto sourceId = Red::TweakDBID();
                        auto foreignType = propInfo->foreignType;
                        auto inlinePath = ComposePath(propPath, itemIndex);

                        if (!ResolveInlineNode(aChangeset, inlinePath, itemData, foreignType, sourceId))
                        {
                            inlineFailed = true;
                            break;
                        }

                        auto inlineName = ComposeInlineName(propName, foreignType, m_path, itemIndex);

                        HandleRecordNode(aChangeset, propMode, inlinePath, inlineName, itemData, foreignType, sourceId);

                        if (overrideData.IsNull())
                        {
                            overrideData = YAML::Clone(originalData);
                        }

                        // Overwrite inline item with foreign key
                        overrideData[itemIndex] = inlineName;
                    }
                }

                if (inlineFailed)
                    continue;
            }
            else if (originalData.IsMap())
            {
                auto sourceId = Red::TweakDBID();
                auto foreignType = propInfo->foreignType;

                if (!ResolveInlineNode(aChangeset, propPath, originalData, foreignType, sourceId))
                    continue;

                auto inlineName = ComposeInlineName(propName, foreignType, m_path);

                // Special handling for UIIcon
                if (propInfo->foreignType->GetName() == UIIconType)
                {
                    // Item records have both .iconPath and .icon properties, but last one is never used.
                    // So if parent record has .iconPath property then autofill it with our inline icon name.
                    if (recordInfo->props.contains("iconPath") && !aNode["iconPath"])
                    {
                        aChangeset.SetFlat(Red::TweakDBID(recordId, ".iconPath"), ResolveFlatType("String"),
                                           Red::MakeInstance<Red::CString>(inlineName.c_str()));
                    }

                    // Then force type prefix to make it accessible by short name that we just set in .iconPath.
                    inlineName.insert(0, "UIIcon.");
                }

                HandleRecordNode(aChangeset, propMode, propPath, inlineName, originalData, foreignType, sourceId);

                // Overwrite inline data with foreign key
                overrideData = inlineName;
            }
        }

        const auto& nodeData = !overrideData.IsNull() ? overrideData : originalData;

        // Array mutations
        if (propInfo->isArray)
        {
            if (HandleMutations(aChangeset, propPath, propName, nodeData, propInfo->elementType))
            {
                if (isOriginalBase)
                {
                    aChangeset.ReinheritFlat(propId, recordId, propInfo->appendix);
                }
                continue;
            }
        }

        const auto propValue = MakeValue(propInfo->type, nodeData);

        if (!propValue)
        {
            LogError("{}.{}: Invalid value, expected \"{}\".", aRecordPath, nodeKey,
                     propInfo->type->GetName().ToString());
            continue;
        }

        aChangeset.SetFlat(propId, propInfo->type, propValue);

        if (isOriginalBase)
        {
            aChangeset.ReinheritFlat(propId, recordId, propInfo->appendix);
        }
    }
}

bool App::YamlReader::ResolveInlineNode(TweakChangeset& aChangeset, const std::string& aPath, const YAML::Node& aNode,
                                        const Red::CClass*& aForeignType, Red::TweakDBID& aSourceId)
{
    if (!CheckConditions(aNode))
        return false;

    {
        if (const auto cloneAttr = aNode[BaseAttrKey]; cloneAttr.IsDefined())
        {
            const auto sourceId = ResolveTweakDBID(cloneAttr);
            const auto sourceType = ResolveRecordInstanceType(aChangeset, sourceId);

            if (sourceType)
            {
                if (sourceType->IsA(aForeignType))
                {
                    aSourceId = sourceId;
                    aForeignType = sourceType;
                    return true;
                }

                LogError("{}: Cannot inline from {}, the record has incompatible type.", aPath, cloneAttr.Scalar());
            }
            else
            {
                LogError("{}: Cannot clone from {}, the record doesn't exist.", aPath, cloneAttr.Scalar());
            }

            return false;
        }
    }

    {
        if (const auto typeAttr = aNode[TypeAttrKey]; typeAttr.IsDefined())
        {
            if (const auto suggestedType = ResolveRecordType(typeAttr))
            {
                if (suggestedType->IsA(aForeignType))
                {
                    aForeignType = suggestedType;
                    return true;
                }

                LogError("{}: Cannot be inlined, provided type {} is incompatible with property type.", aPath,
                         typeAttr.Scalar());
            }
            else
            {
                LogError("{}: Cannot be inlined, provided type {} is not a known record type or abstract.", aPath,
                         typeAttr.Scalar());
            }

            return false;
        }
    }

    return true;
}

bool App::YamlReader::HandleMutations(TweakChangeset& aChangeset, const std::string& aPath, const std::string& aName,
                                      const YAML::Node& aNode, const Red::CBaseRTTIType* aElementType)
{
    if (!aNode.IsSequence())
        return false;

    const auto flatId = Red::TweakDBID(aName);

    bool isMutation = false;
    bool isAssignment = false;

    for (std::size_t itemIndex = 0; itemIndex < aNode.size(); ++itemIndex)
    {
        auto itemData = aNode[itemIndex];

        if (itemData.Tag().length() <= 1)
        {
            isAssignment = true;
            continue;
        }

        switch (const auto tag = Red::FNV1a64(itemData.Tag().c_str()))
        {
        case AppendOp:
        case AppendOnceOp:
        {
            const auto itemValue = MakeValue(aElementType, itemData);

            if (!itemValue)
            {
                LogError("{}.{}: Invalid value, expected \"{}\".", aPath, std::to_string(itemIndex),
                         aElementType->GetName().ToString());
                continue;
            }

            aChangeset.AppendElement(flatId, aElementType, itemValue, tag == AppendOnceOp);
            isMutation = true;
            break;
        }
        case PrependOp:
        case PrependOnceOp:
        {
            const auto itemValue = MakeValue(aElementType, itemData);

            if (!itemValue)
            {
                LogError("{}.{}: Invalid value, expected \"{}\".", aPath, std::to_string(itemIndex),
                         aElementType->GetName().ToString());
                continue;
            }

            aChangeset.PrependElement(flatId, aElementType, itemValue, tag == PrependOnceOp);
            isMutation = true;
            break;
        }
        case MergeOp:
        case AppendFromOp:
        {
            const auto sourceId = ResolveTweakDBID(itemData);

            if (!sourceId.IsValid())
            {
                LogError("{}.{}: Invalid value, expected \"TweakDBID\".", aPath, std::to_string(itemIndex));
                continue;
            }

            aChangeset.AppendFrom(flatId, sourceId);
            isMutation = true;
            break;
        }
        case PrependFromOp:
        {
            const auto sourceId = ResolveTweakDBID(itemData);

            if (!sourceId.IsValid())
            {
                LogError("{}.{}: Invalid value, expected \"TweakDBID\".", aPath, std::to_string(itemIndex));
                continue;
            }

            aChangeset.PrependFrom(flatId, sourceId);
            isMutation = true;
            break;
        }
        case RemoveOp:
        {
            const auto itemValue = MakeValue(aElementType, itemData);

            if (!itemValue)
            {
                LogError("{}.{}: Invalid value, expected \"{}\".", aPath, std::to_string(itemIndex),
                         aElementType->GetName().ToString());
                continue;
            }

            aChangeset.RemoveElement(flatId, aElementType, itemValue);
            isMutation = true;
            break;
        }
        case RemoveAllOp:
        {
            aChangeset.RemoveAllElements(flatId);
            isMutation = true;
            break;
        }
        default:
        {
            LogError("{}.{}: Invalid action {}.", aPath, std::to_string(itemIndex), itemData.Tag());
        }
        }
    }

    if (isMutation && isAssignment)
    {
        LogWarning("{}: Mixed definition of array replacement and mutations. "
                   "Only mutations will take effect.",
                   aPath);
    }

    return isMutation;
}

void App::YamlReader::UpdateFlatOwner(TweakChangeset& aChangeset, const std::string& aName)
{
    const auto separatorPos = aName.find_last_of(PropSeparator);

    if (separatorPos != std::string::npos)
    {
        const auto recordName = aName.substr(0, separatorPos);
        const auto recordId = Red::TweakDBID(recordName);

        if (ResolveRecordInstanceType(aChangeset, recordId))
        {
            aChangeset.UpdateRecord(recordId);

            if (IsOriginalBaseRecord(recordId))
            {
                aChangeset.ReinheritFlat(aName.data(), recordId, aName.substr(separatorPos));
            }
        }
    }
}

bool App::YamlReader::CheckConditions(const YAML::Node& aNode) const
{
    if (const auto& gameConditionAttr = aNode[GameConditionKey]; gameConditionAttr.IsDefined())
    {
        if (!gameConditionAttr.IsScalar())
            return false;

        if (!m_context->CheckGameVersion(gameConditionAttr.Scalar()))
            return false;
    }

    if (const auto& dlcConditionAttr = aNode[DLCConditionKey]; dlcConditionAttr.IsDefined())
    {
        if (!dlcConditionAttr.IsScalar())
            return false;

        if (!m_context->CheckInstalledDLC(dlcConditionAttr.Scalar()))
            return false;
    }

    return true;
}

App::YamlReader::PropertyMode App::YamlReader::ResolvePropertyMode(const YAML::Node& aNode, const PropertyMode aDefault)
{
    if (const auto& modeAttr = aNode[PropModeKey]; modeAttr.IsDefined() && modeAttr.Scalar() == PropModeAuto)
    {
        return PropertyMode::Auto;
    }

    return aDefault;
}

App::TweakTypeSpecPtr App::YamlReader::ResolvePropertyFlatInfo(const YAML::Node& aNode)
{
    if (!aNode.IsScalar())
        return nullptr;

    return GetTweakTypeSpec(aNode.Scalar());
}

const Red::CBaseRTTIType* App::YamlReader::ResolveFlatType(const YAML::Node& aNode)
{
    return Red::GetFlatType(aNode.Scalar().c_str());
}

const Red::CBaseRTTIType* App::YamlReader::ResolveFlatType(Red::CName aName)
{
    return Red::GetFlatType(aName);
}

const Red::CClass* App::YamlReader::ResolveRecordType(const YAML::Node& aNode)
{
    return Red::GetRecordType(aNode.Scalar().c_str());
}

Red::TweakDBID App::YamlReader::ResolveTweakDBID(const YAML::Node& aNode)
{
    if (!aNode.IsDefined())
        return {};

    const auto resolvedId = ConvertValue<Red::TweakDBID>(aNode);

    if (!resolvedId)
        return {};

    return *resolvedId;
}
