#include "YamlReader.hpp"

namespace
{
constexpr auto InstanceAttrKey = "$instances";

constexpr auto AttrMark = '$';
constexpr auto AttrOpen = "({";
constexpr auto AttrClose = ")}";

struct InstanceValue
{
    const std::string value;
    const YAML::Node node;
};

using InstanceData = Core::Map<uint64_t, InstanceValue>;

uint64_t MakeKey(const std::string& aName)
{
    return Red::FNV1a64(aName.c_str());
}

uint64_t MakeKey(const char* aName, uint32_t aSize)
{
    return Red::FNV1a64(reinterpret_cast<const uint8_t*>(aName), aSize);
}

std::string FormatString(const std::string& aInput, const InstanceData& aData)
{
    constexpr auto MaxLength = 512;

    char buffer[MaxLength + 1];
    char* out = buffer;
    const char* max = buffer + MaxLength;
    const char* str = aInput.c_str();

    while (str && *str)
    {
        auto* attrOpen = strchr(str, AttrMark);

        if (!attrOpen)
        {
            if (out == buffer)
            {
                return aInput;
            }

            while (*str && out < max)
            {
                *out = *str;
                ++out;
                ++str;
            }
            break;
        }

        int attCloseChr;
        if (*(attrOpen + 1) == AttrOpen[0])
        {
            attCloseChr = AttrClose[0];
        }
        else if (*(attrOpen + 1) == AttrOpen[1])
        {
            attCloseChr = AttrClose[1];
        }
        else
        {
            *out = *str;
            ++out;
            ++str;
            continue;
        }

        auto* attrClose = strchr(str, attCloseChr);

        if (!attrClose)
        {
            return aInput;
        }

        while (str != attrOpen && out < max)
        {
            *out = *str;
            ++out;
            ++str;
        }

        if (out == max)
        {
            break;
        }

        str = attrClose + 1;

        const auto attr = MakeKey(attrOpen + 2, attrClose - attrOpen - 2);
        const char* value = nullptr;

        const auto it = aData.find(attr);
        if (it != aData.end())
        {
            value = it.value().value.c_str();
        }

        if (value)
        {
            while (value && *value && out < max)
            {
                *out = *value;
                ++out;
                ++value;
            }

            if (out == max)
            {
                break;
            }
        }
    }

    *out = '\0';

    return buffer;
}

void FormatNode(const YAML::Node& aNode, const InstanceData& aData)
{
    switch (aNode.Type())
    {
    case YAML::NodeType::Scalar:
    {
        const auto& value = aNode.Scalar();
        const auto markPos = value.find(AttrMark);
        if (markPos != std::string::npos)
        {
            if (markPos == 0)
            {
                const auto attr = MakeKey(value.data() + 2, value.size() - 3);
                const auto it = aData.find(attr);
                if (it != aData.end())
                {
                    const_cast<YAML::Node&>(aNode) = it.value().node;
                    return;
                }
            }

            auto node = YAML::Node(FormatString(value, aData));
            node.SetTag(aNode.Tag());

            const_cast<YAML::Node&>(aNode) = node;
        }
        break;
    }
    case YAML::NodeType::Map:
    {
        for (auto& nodeIt : aNode)
        {
            FormatNode(nodeIt.second, aData);
        }
        break;
    }
    case YAML::NodeType::Sequence:
    {
        for (std::size_t i = 0; i < aNode.size(); ++i)
        {
            FormatNode(aNode[i], aData);
        }
        break;
    }
    default:
        break;
    }
}
}

void App::YamlReader::ProcessTemplates(YAML::Node& aRootNode)
{
    Core::Vector<std::pair<const std::string&, YAML::Node>> templates;

    for (const auto& nodeIt : aRootNode)
    {
        const auto& subKey = nodeIt.first.Scalar();
        const auto& subNode = nodeIt.second;

        if (!subNode.IsMap() || !subNode[InstanceAttrKey].IsDefined())
            continue;

        templates.emplace_back(subKey, subNode);
    }

    for (auto& [templateName, templateNode] : templates)
    {
        aRootNode.remove(templateName);

        const auto& dataNode = templateNode[InstanceAttrKey];

        if (!dataNode.IsSequence())
        {
            LogError("{}: Template instances must be an array of structs.", templateName);
            continue;
        }

        templateNode.remove(InstanceAttrKey);

        for (std::size_t i = 0; i < dataNode.size(); ++i)
        {
            InstanceData instanceData;

            for (const auto& propIt : dataNode[i])
            {
                const auto propKey = MakeKey(propIt.first.Scalar());
                const auto& propValue = propIt.second;

                if (propValue.IsScalar())
                {
                    instanceData.insert({propKey, {propValue.Scalar(), propValue}});
                }
                else
                {
                    instanceData.insert({propKey, {"", propValue}});
                }
            }

            auto instanceName = FormatString(templateName, instanceData);

            if (aRootNode[instanceName].IsDefined())
            {
                LogError("{}: Cannot create instance {}, because it already exists.",
                         templateName, instanceName);
                continue;
            }

            auto instanceNode = YAML::Clone(templateNode);
            FormatNode(instanceNode, instanceData);

            aRootNode[instanceName] = instanceNode;
        }
    }
}
