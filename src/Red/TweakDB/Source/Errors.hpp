#pragma once

#include "Red/TweakDB/Source/Grammar.hpp"

#define RED_TWEAK_ERROR(rule, msg) template<> static constexpr auto message<rule> = msg;

namespace Red
{
struct TweakError
{
    template<typename>
    static constexpr const char* message = nullptr;

    RED_TWEAK_ERROR(TweakGrammar::package_name, "Expected package name");
    RED_TWEAK_ERROR(TweakGrammar::using_name, "Expected package name");

    RED_TWEAK_ERROR(TweakGrammar::group_tag_name, "Expected tag name");
    RED_TWEAK_ERROR(TweakGrammar::group_tag_sfx, "Expected ']'");
    RED_TWEAK_ERROR(TweakGrammar::group_name, "Expected group name");
    RED_TWEAK_ERROR(TweakGrammar::group_base, "Expected group name");
    RED_TWEAK_ERROR(TweakGrammar::group_begin, "Expected '{'"); // if pos == operator, then missing flat type
    RED_TWEAK_ERROR(TweakGrammar::group_end, "Expected '}'");

    RED_TWEAK_ERROR(TweakGrammar::inline_base, "Expected group name");
    RED_TWEAK_ERROR(TweakGrammar::inline_end, "Expected '}'");

    RED_TWEAK_ERROR(TweakGrammar::flat_name, "Expected flat name");
    RED_TWEAK_ERROR(TweakGrammar::flat_op, "Expected assignment operator");
    RED_TWEAK_ERROR(TweakGrammar::flat_value, "Invalid value");
    RED_TWEAK_ERROR(TweakGrammar::flat_end, "Expected ';'");

    RED_TWEAK_ERROR(TweakGrammar::array_item, "Invalid array item");
    RED_TWEAK_ERROR(TweakGrammar::array_sep, "Expected ','");
    RED_TWEAK_ERROR(TweakGrammar::array_end, "Expected ']'");

    RED_TWEAK_ERROR(TweakGrammar::source_with_package_member, "Expected group or flat definition");
    RED_TWEAK_ERROR(TweakGrammar::source_no_package_member, "Expected group definition");
    RED_TWEAK_ERROR(TweakGrammar::source, "Invalid tweak file");
};
}
