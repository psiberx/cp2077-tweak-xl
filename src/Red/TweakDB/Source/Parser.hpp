#pragma once

#include "Red/TweakDB/Source/Grammar.hpp"
#include "Red/TweakDB/Source/Errors.hpp"
#include "Red/TweakDB/Source/Source.hpp"

namespace Red
{
class TweakParser
{
public:
    static Core::SharedPtr<TweakSource> Parse(const std::filesystem::path& aPath);

private:
    struct ParseState
    {
        Core::Vector<std::string> tags;
        Core::SharedPtr<TweakGroup> group;
        Core::SharedPtr<TweakFlat> flat;
        Core::SharedPtr<TweakValue> value;

        using Parent = std::pair<Core::SharedPtr<TweakGroup>, Core::SharedPtr<TweakFlat>>;
        Core::Vector<Parent> nested;
        Core::SharedPtr<TweakGroup> closed;

        std::string flatType;
        std::string foreignType;
        bool isArray = false;
        bool hasType = false;
    };

    template<typename Rule>
    using ParseControl = tao::pegtl::must_if<TweakError>::control<Rule>;

    template<typename Rule>
    struct ParseAction {};

    static ETweakFlatType ResolveType(const std::string& aInput);
    static ETweakFlatOp ResolveOperation(const std::string& aInput);

    static std::string FormatError(const std::filesystem::path& aPath, const tao::pegtl::position& aPosition,
                                   const std::string_view& aMessage);
};
}
