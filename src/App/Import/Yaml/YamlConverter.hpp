#pragma once

#include "stdafx.hpp"

namespace App
{
class YamlConverter
{
public:
    template<typename T>
    Core::SharedPtr<T> Convert(const YAML::Node& aNode, bool aStrict = false);

    template<typename E>
    Core::SharedPtr<RED4ext::DynArray<E>> ConvertArray(const YAML::Node& aNode, bool aStrict = false);

    template<typename T>
    bool Convert(const YAML::Node& aNode, Core::SharedPtr<void>& aValue, bool aStrict = false);

    template<typename E>
    bool ConvertArray(const YAML::Node& aNode, Core::SharedPtr<void>& aValue, bool aStrict = false);

    Core::SharedPtr<void> Convert(const RED4ext::CBaseRTTIType* aType, const YAML::Node& aNode);
    Core::SharedPtr<void> Convert(RED4ext::CName aTypeName, const YAML::Node& aNode);
    std::pair<RED4ext::CName, Core::SharedPtr<void>> Convert(const YAML::Node& aNode);
};
}
