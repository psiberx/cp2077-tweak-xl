#pragma once

namespace App
{
class YamlConverter
{
public:
    template<typename T>
    Core::SharedPtr<T> Convert(const YAML::Node& aNode, bool aStrict = false);

    template<typename E>
    Core::SharedPtr<Red::DynArray<E>> ConvertArray(const YAML::Node& aNode, bool aStrict = false);

    template<typename T>
    bool Convert(const YAML::Node& aNode, Core::SharedPtr<void>& aValue, bool aStrict = false);

    template<typename E>
    bool ConvertArray(const YAML::Node& aNode, Core::SharedPtr<void>& aValue, bool aStrict = false);

    Core::SharedPtr<void> Convert(const Red::CBaseRTTIType* aType, const YAML::Node& aNode);
    Core::SharedPtr<void> Convert(Red::CName aTypeName, const YAML::Node& aNode);
    std::pair<Red::CName, Core::SharedPtr<void>> Convert(const YAML::Node& aNode);
};
}
