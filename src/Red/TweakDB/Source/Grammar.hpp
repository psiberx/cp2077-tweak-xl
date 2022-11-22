#pragma once

namespace Red::TweakGrammar
{
using namespace tao::pegtl;

/* Control */

template<typename... R>
using unless = not_at<try_catch<R...>>;

/* Comments */

using comment_line = seq<two<'/'>, until<eolf>>;
using comment_block = seq<string<'/', '*'>, until<string<'*', '/'>>>;
using comment = sor<comment_line, comment_block>;

/* Separators */

using _ = star<sor<space, comment>>;
using space = plus<sor<blank, comment>>;
using comma = seq<_, one<','>, _>;

/* Basic identifiers */

using name_first = sor<ranges<'a', 'z', 'A', 'Z'>, one<'_'>>;
using name_other = sor<ranges<'a', 'z', 'A', 'Z', '0', '9'>, one<'_'>>;
using name = seq<name_first, star<name_other>>;

using path_first = sor<ranges<'a', 'z', 'A', 'Z'>, one<'_'>>;
using path_other = sor<ranges<'a', 'z', 'A', 'Z', '0', '9'>, one<'_', '.'>>;
using path = seq<path_first, star<path_other>>;

using hash = seq<string<'0', 'x'>, rep_min_max<9, 10, xdigit>>;

/* Complex identifiers */

using package_id = sor<hash, name>;
using group_id = sor<hash, path>;
using group_path = seq<package_id, one<'.'>, group_id>;

/* Package statement */

using package_keyword = string<'p', 'a', 'c', 'k', 'a', 'g', 'e'>;
struct package_name : package_id {};
struct package_stmt : seq<package_keyword, space, package_name> {};

/* Using statement */

using using_keyword = string<'u', 's', 'i', 'n', 'g'>;
using using_sep = comma;
struct using_name : package_id {};
struct using_stmt : seq<using_keyword, space, using_name, star<using_sep, using_name>> {};

/* Group statement */

struct flat_stmt;

struct group_tag_pfx : one<'['> {};
struct group_tag_sfx : one<']'> {};
struct group_tag_name : name {};
struct group_tag : seq<group_tag_pfx, _, group_tag_name, _, group_tag_sfx> {};
struct group_tags : star<group_tag, _> {};
struct group_name : group_id {};
struct group_base : sor<group_path, group_id> {};
struct group_inherit : seq<one<':'>, _, group_base> {};
struct group_begin : one<'{'> {};
struct group_end : one<'}'> {};
struct group_stmt : seq<group_tags, group_name, opt<_, group_inherit>, _, group_begin, star<_, flat_stmt>, _, group_end> {};

/* Inline expression */

struct inline_begin : one<'{'> {};
struct inline_end : one<'}'> {};
struct inline_base : sor<group_path, group_id> {};
struct inline_inherit : seq<one<':'>, _, inline_base> {};
struct inline_expr : seq<inline_begin, star<_, flat_stmt>, _, inline_end, opt<_, inline_inherit>> {};

/* Flat statement */

using type_int = string<'i', 'n', 't'>;
using type_float = string<'f', 'l', 'o', 'a', 't'>;
using type_bool = string<'b', 'o', 'o', 'l'>;
using type_string = string<'s', 't', 'r', 'i', 'n', 'g'>;
using type_cname = string<'C', 'N', 'a', 'm', 'e'>;
using type_res = string<'R', 'e', 's', 'R', 'e', 'f'>;
using type_loc = string<'L', 'o', 'c', 'K', 'e', 'y'>;
using type_quat = string<'Q', 'u', 'a', 't', 'e', 'r', 'n', 'i', 'o', 'n'>;
using type_euler = string<'E', 'u', 'l', 'e', 'r', 'A', 'n', 'g', 'l', 'e', 's'>;
using type_vec3 = string<'V', 'e', 'c', 't', 'o', 'r', '3'>;
using type_vec2 = string<'V', 'e', 'c', 't', 'o', 'r', '2'>;
using type_color = string<'C', 'o', 'l', 'o', 'r'>;

struct type_scalar : sor<type_int, type_float, type_bool, type_string, type_cname, type_res, type_loc> {};
struct type_struct : sor<type_quat, type_euler, type_vec3, type_vec2, type_color> {};
struct type_fk_pfx : string<'f', 'k', '<'> {};
struct type_fk_sfx : one<'>'> {};
struct type_fk_name : name {};
struct type_fk : seq<type_fk_pfx, _, type_fk_name, _, type_fk_sfx> {};
struct type_array_sfx : string<'[', ']'> {};
struct type_base_name : sor<type_scalar, type_struct, type_fk> {};
struct type_expr : seq<type_base_name, opt<type_array_sfx>> {};

using bool_true = string<'t', 'r', 'u', 'e'>;
using bool_false = string<'f', 'a', 'l', 's', 'e'>;
using float_sfx = one<'f'>;

struct scalar_bool : sor<bool_true, bool_false> {};
struct scalar_number : seq<opt<one<'-'>>, sor<seq<one<'.'>, plus<digit>>, seq<plus<digit>, opt<one<'.'>, star<digit>>>>, opt<float_sfx>> {};
struct scalar_string : seq<one<'"'>, until<at<one<'"'>>, any>, any> {};
struct scalar_expr : sor<scalar_bool, scalar_number, scalar_string> {};

struct struct_begin : one<'('> {};
struct struct_end : one<')'> {};
struct struct_sep : comma {};
struct struct_element : scalar_number {};
struct struct_expr : seq<struct_begin, _, struct_element, struct_sep, struct_element, rep_max<2, struct_sep, struct_element>, _, struct_end> {};

struct array_begin : one<'['> {};
struct array_end : one<']'> {};
struct array_sep : seq<one<','>, _> {};
struct array_item : sor<scalar_expr, struct_expr, inline_expr> {};
struct array_expr : seq<array_begin, _, star<unless<array_end>, array_item, _, sor<seq<unless<array_end>, array_sep>, at<array_end>>>, array_end> {};

using op_assign = one<'='>;
using op_append = string<'+', '='>;
using op_remove = string<'-', '='>;

struct flat_type : seq<type_expr, space> {};
struct flat_name : name {};
struct flat_op : sor<op_assign, op_append, op_remove> {};
struct flat_value : sor<scalar_expr, struct_expr, array_expr, inline_expr> {};
struct flat_end : one<';'> {};
struct flat_stmt : seq<opt<flat_type>, unless<group_end>, flat_name, _, flat_op, _, flat_value, _, flat_end> {};
struct flat_decl_start : seq<flat_type> {};
struct flat_decl_continue : seq<flat_name, _, flat_op, _, flat_value, _, flat_end> {};

/* Complete source */

struct source_with_package_member : if_then_else<flat_decl_start, flat_decl_continue, group_stmt> {};
struct source_with_package_body : star<not_at<eof>, source_with_package_member, _> {};
struct source_with_package : seq<package_stmt, _, opt<using_stmt, _>, source_with_package_body> {};

struct source_no_package_member : group_stmt {};
struct source_no_package_body : star<not_at<eof>, source_no_package_member, _> {};
struct source_no_package : seq<not_at<package_stmt>, source_no_package_body> {};

struct source : seq<_, sor<source_with_package, source_no_package>, eof> {};

/* Constants */

namespace internal
{
    template<typename>
    struct to_const;

    template<template<char...> class X, char... Cs>
    struct to_const<X<Cs...>>
    {
        static constexpr const char value[] = {Cs..., 0};
    };
}

namespace Type
{
constexpr auto Int = internal::to_const<type_int>::value;
constexpr auto Float = internal::to_const<type_float>::value;
constexpr auto Bool = internal::to_const<type_bool>::value;
constexpr auto String = internal::to_const<type_string>::value;
constexpr auto CName = internal::to_const<type_cname>::value;
constexpr auto ResRef = internal::to_const<type_res>::value;
constexpr auto LocKey = internal::to_const<type_loc>::value;
constexpr auto Quaternion = internal::to_const<type_quat>::value;
constexpr auto EulerAngles = internal::to_const<type_euler>::value;
constexpr auto Vector3 = internal::to_const<type_vec3>::value;
constexpr auto Vector2 = internal::to_const<type_vec2>::value;
constexpr auto Color = internal::to_const<type_color>::value;
}

namespace Bool
{
constexpr auto True = internal::to_const<bool_true>::value;
constexpr auto False = internal::to_const<bool_false>::value;
}

namespace Float
{
constexpr auto Suffix = internal::to_const<float_sfx>::value;
}

namespace Op
{
constexpr auto Assign = internal::to_const<op_assign>::value;
constexpr auto Append = internal::to_const<op_append>::value;
constexpr auto Remove = internal::to_const<op_remove>::value;
}
}
