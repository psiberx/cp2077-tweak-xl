#include "YamlReader.hpp"

namespace
{
constexpr auto InstanceAttrKey = "$instances";
constexpr auto ValueAttrKey = "$value";

constexpr auto AttrMark = '$';
constexpr auto AttrOpen = "({";
constexpr auto AttrClose = ")}";

using InstanceData = Core::Map<uint64_t, YAML::Node>;
const InstanceData s_blankInstanceData;

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
            if (it.value().IsScalar())
            {
                value = it.value().Scalar().c_str();
            }
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

void PrepareInstanceData(InstanceData& aInstanceData, const YAML::Node& aInstanceDataNode)
{
    for (const auto& propIt : aInstanceDataNode)
    {
        const auto propKey = MakeKey(propIt.first.Scalar());
        const auto& propValue = propIt.second;

        aInstanceData[propKey] = propValue;
    }
}

void ProcessNode(YAML::Node& aNode, const InstanceData& aInstanceData)
{
    switch (aNode.Type())
    {
    case YAML::NodeType::Scalar:
    {
        const auto& value = aNode.Scalar();
        const auto markPos = value.find(AttrMark);
        if (markPos != std::string::npos)
        {
            YAML::Node expandedNode{YAML::NodeType::Undefined};

            if (markPos == 0)
            {
                const auto attr = MakeKey(value.data() + 2, value.size() - 3);
                auto it = aInstanceData.find(attr);
                if (it != aInstanceData.end())
                {
                    expandedNode = YAML::Clone(it->second);
                }
            }

            if (!expandedNode.IsDefined())
            {
                expandedNode = FormatString(value, aInstanceData);
            }

            expandedNode.SetTag(aNode.Tag());

            aNode = expandedNode;
        }
        break;
    }
    case YAML::NodeType::Map:
    {
        for (auto nodeIt : aNode)
        {
            ProcessNode(nodeIt.second, aInstanceData);
        }
        break;
    }
    case YAML::NodeType::Sequence:
    {
        if (aNode.size() == 0)
            break;

        YAML::Node expandedNode{YAML::NodeType::Undefined};

        for (std::size_t i = 0; i < aNode.size(); ++i)
        {
            auto subNode = aNode[i];

            if (subNode.IsMap())
            {
                auto instanceListNode = subNode[InstanceAttrKey];

                if (instanceListNode.IsDefined())
                {
                    const_cast<YAML::Node&>(subNode).remove(InstanceAttrKey);

                    if (instanceListNode.IsSequence())
                    {
                        if (!expandedNode.IsDefined())
                        {
                            expandedNode = YAML::Node{YAML::NodeType::Sequence};

                            for (std::size_t j = 0; j < i; ++j)
                            {
                                expandedNode.push_back(aNode[j]);
                            }
                        }

                        for (std::size_t j = 0; j < instanceListNode.size(); ++j)
                        {
                            InstanceData instanceData{aInstanceData};
                            PrepareInstanceData(instanceData, instanceListNode[j]);

                            auto instanceNode = YAML::Clone(subNode);
                            ProcessNode(instanceNode, instanceData);

                            auto valueNode = instanceNode[ValueAttrKey];
                            if (valueNode.IsDefined() && !valueNode.IsMap())
                            {
                                if (valueNode.Tag() == "?")
                                {
                                    valueNode.SetTag(subNode.Tag());
                                }

                                expandedNode.push_back(valueNode);
                            }
                            else
                            {
                                expandedNode.push_back(instanceNode);
                            }
                        }

                        continue;
                    }
                }
            }

            ProcessNode(subNode, aInstanceData);

            if (expandedNode.IsDefined())
            {
                expandedNode.push_back(subNode);
            }
        }

        if (expandedNode.IsDefined())
        {
            const_cast<YAML::Node&>(aNode) = expandedNode;
        }
        break;
    }
    }
}
}

void App::YamlReader::ProcessTemplates(YAML::Node& aRootNode)
{
    auto processedTopNodes = 0;

    {
        auto hasTopTemplates = false;

        for (auto topNodeIt : aRootNode)
        {
            auto topNode = topNodeIt.second;

            if (topNode.IsMap())
            {
                if (topNode[InstanceAttrKey].IsDefined())
                {
                    hasTopTemplates = true;
                    break;
                }
            }

            ProcessNode(topNode, s_blankInstanceData);
            ++processedTopNodes;
        }

        if (!hasTopTemplates)
            return;
    }

    YAML::Node expandedNode{YAML::NodeType::Map};

    for (auto topNodeIt : aRootNode)
    {
        const auto& topKey = topNodeIt.first.Scalar();
        auto topNode = topNodeIt.second;

        if (processedTopNodes > 0)
        {
            expandedNode.force_insert(topKey, topNode);
            --processedTopNodes;
            continue;
        }

        if (!topNode.IsMap())
        {
            ProcessNode(topNode, s_blankInstanceData);
            expandedNode.force_insert(topKey, topNode);
            continue;
        }

        const auto& instanceListNode = topNode[InstanceAttrKey];

        if (!instanceListNode.IsDefined())
        {
            ProcessNode(topNode, s_blankInstanceData);
            expandedNode.force_insert(topKey, topNode);
            continue;
        }

        if (!instanceListNode.IsSequence())
        {
            ProcessNode(topNode, s_blankInstanceData);
            expandedNode.force_insert(topKey, topNode);
            continue;
        }

        const_cast<YAML::Node&>(topNode).remove(InstanceAttrKey);

        for (std::size_t i = 0; i < instanceListNode.size(); ++i)
        {
            InstanceData instanceData;
            PrepareInstanceData(instanceData, instanceListNode[i]);

            auto instanceName = FormatString(topKey, instanceData);
            auto instanceNode = YAML::Clone(topNode);
            ProcessNode(instanceNode, instanceData);

            expandedNode.force_insert(instanceName, instanceNode);
        }
    }

    aRootNode = expandedNode;
}
