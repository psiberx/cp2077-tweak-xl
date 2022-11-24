#pragma once

namespace App
{
class YamlConverter
{
public:
    template<typename T>
    Core::SharedPtr<T> Convert(const YAML::Node& aNode, bool aStrict = false);

    template<typename T>
    bool Convert(const YAML::Node& aNode, Red::InstancePtr<>& aValue, bool aStrict = false);

    template<typename E>
    Core::SharedPtr<Red::DynArray<E>> ConvertArray(const YAML::Node& aNode, bool aStrict = false);

    template<typename E>
    bool ConvertArray(const YAML::Node& aNode, Red::InstancePtr<>& aValue, bool aStrict = false);

    Red::InstancePtr<> Convert(Red::CName aTypeName, const YAML::Node& aNode);
    Red::InstancePtr<> Convert(const Red::CBaseRTTIType* aType, const YAML::Node& aNode);
    std::pair<Red::CName, Red::InstancePtr<>> Convert(const YAML::Node& aNode);
};
}
