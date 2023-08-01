#include "YamlReader.hpp"

namespace
{
constexpr auto InstanceAttrKey = "$instances";

constexpr auto AttrMark = '$';
constexpr auto AttrOpen = '{';
constexpr auto AttrClose = '}';

uint64_t MakeKey(const std::string& aName)
{
    return Red::FNV1a64(aName.c_str());
}

uint64_t MakeKey(const char* aName, uint32_t aSize)
{
    return Red::FNV1a64(reinterpret_cast<const uint8_t*>(aName), aSize);
}

std::string FormatString(const std::string& aInput, const Core::Map<uint64_t, std::string>& aData)
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

        if (*(attrOpen + 1) != AttrOpen)
        {
            *out = *str;
            ++out;
            ++str;
            continue;
        }

        auto* attrClose = strchr(str, AttrClose);

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
            value = it.value().c_str();
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

YAML::Node FormatNode(const YAML::Node& aNode, const Core::Map<uint64_t, std::string>& aData)
{
    switch (aNode.Type())
    {
    case YAML::NodeType::Scalar:
    {
        const auto& value = aNode.Scalar();
        if (value.find(AttrMark) != std::string::npos)
        {
            return YAML::Node(FormatString(value, aData));
        }
        else
        {
            return aNode;
        }
    }
    case YAML::NodeType::Map:
    {
        YAML::Node node;
        for (auto& nodeIt : aNode)
        {
            node[nodeIt.first] = FormatNode(nodeIt.second, aData);
        }
        return node;
    }
    case YAML::NodeType::Sequence:
    {
        YAML::Node node;
        for (std::size_t i = 0; i < aNode.size(); ++i)
        {
            node[i] = FormatNode(aNode[i], aData);
        }
        return node;
    }
    default:
        return aNode;
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
            Core::Map<uint64_t, std::string> instanceData;

            for (const auto& propIt : dataNode[i])
            {
                const auto propKey = MakeKey(propIt.first.Scalar());
                const auto& propValue = propIt.second;

                if (!propValue.IsScalar())
                    continue;

                instanceData[propKey] = propValue.Scalar();
            }

            auto instanceName = FormatString(templateName, instanceData);

            if (aRootNode[instanceName].IsDefined())
            {
                LogError("{}: Cannot create instance {}, because it already exists.",
                         templateName, instanceName);
                continue;
            }

            auto instanceNode = FormatNode(templateNode, instanceData);

            aRootNode[instanceName] = instanceNode;
        }
    }
}
