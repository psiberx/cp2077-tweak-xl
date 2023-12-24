#include "Parser.hpp"

/* Package */

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::package_name>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        package.package = in.string();
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::using_name>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        package.usings.push_back(in.string());
    }
};

/* Tags */

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::tag_name>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.tags.push_back(in.string());
    }
};

/* Groups */

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::group_name>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        auto group = Core::MakeShared<TweakGroup>();
        group->name = in.string();
        group->tags.swap(state.tags);

        package.groups.push_back(group);

        state.group = group;
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::group_base>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.group->base = in.string();
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::group_end>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.group.reset();
        state.closed.reset();
        state.nested.clear();
    }
};

/* Flats */

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::type_base_name>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.flatType = in.string();
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::type_fk_name>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.foreignType = in.string();
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::type_array_sfx>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.isArray = true;
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::flat_type>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.hasType = true;
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::flat_name>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        auto flat = Core::MakeShared<TweakFlat>();
        flat->name = in.string();
        flat->tags.swap(state.tags);

        if (state.hasType)
        {
            if (!state.foreignType.empty())
            {
                flat->type = ETweakFlatType::ForeignKey;
                flat->foreignType = state.foreignType;
            }
            else
            {
                flat->type = ResolveType(state.flatType);
            }

            flat->isArray = state.isArray;
        }

        if (state.group)
        {
            state.group->flats.push_back(flat);
        }
        else
        {
            package.flats.push_back(flat);
        }

        state.flat = flat;

        state.flatType = "";
        state.foreignType = "";
        state.isArray = false;
        state.hasType = false;
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::flat_op>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.flat->operation = ResolveOperation(in.string());
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::scalar_bool>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        auto value = Core::MakeShared<TweakValue>();
        value->type = ETweakValueType::Bool;
        value->data.push_back(in.string());

        state.flat->values.push_back(value);
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::scalar_number>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        auto value = Core::MakeShared<TweakValue>();
        value->type = ETweakValueType::Number;
        value->data.push_back(in.string());

        state.flat->values.push_back(value);
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::scalar_string>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        auto value = Core::MakeShared<TweakValue>();
        value->type = ETweakValueType::String;

        const auto& str = in.string();
        value->data.push_back(str.substr(1, str.size() - 2));

        state.flat->values.push_back(value);
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::struct_begin>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        auto value = Core::MakeShared<TweakValue>();
        value->type = ETweakValueType::Struct;

        state.flat->values.push_back(value);

        state.value = value;
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::struct_element>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.value->data.push_back(in.string());
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::struct_end>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.value.reset();
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::flat_end>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.flat.reset();
    }
};

/* Inlines */

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::inline_begin>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        auto group = Core::MakeShared<TweakGroup>();

        auto inlined = Core::MakeShared<TweakInline>();
        inlined->owner = state.nested.empty() ? state.group : state.nested.front().first;
        inlined->parent = state.group;
        inlined->group = group;

        package.inlines.push_back(inlined);

        auto value = Core::MakeShared<TweakValue>();
        value->type = ETweakValueType::Inline;
        value->group = group;

        state.flat->values.push_back(value);

        state.nested.push_back({state.group, state.flat});
        state.group = group;
        state.flat.reset();
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::inline_end>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.closed = state.group;
        state.group = state.nested.back().first;
        state.flat = state.nested.back().second;
        state.nested.pop_back();
    }
};

template<>
struct Red::TweakParser::ParseAction<Red::TweakGrammar::inline_base>
{
    template<typename ParseInput>
    static void apply(const ParseInput& in, ParseState& state, TweakSource& package)
    {
        state.closed->base = in.string();
    }
};

/***/

Red::ETweakFlatType Red::TweakParser::ResolveType(const std::string& aInput)
{
    switch (CName(aInput.c_str()))
    {
    case CName(TweakGrammar::Type::Int): return ETweakFlatType::Int;
    case CName(TweakGrammar::Type::Float): return ETweakFlatType::Float;
    case CName(TweakGrammar::Type::Bool): return ETweakFlatType::Bool;
    case CName(TweakGrammar::Type::String): return ETweakFlatType::String;
    case CName(TweakGrammar::Type::CName): return ETweakFlatType::CName;
    case CName(TweakGrammar::Type::ResRef): return ETweakFlatType::ResRef;
    case CName(TweakGrammar::Type::LocKey): return ETweakFlatType::LocKey;
    case CName(TweakGrammar::Type::Quaternion): return ETweakFlatType::Quaternion;
    case CName(TweakGrammar::Type::EulerAngles): return ETweakFlatType::EulerAngles;
    case CName(TweakGrammar::Type::Vector3): return ETweakFlatType::Vector3;
    case CName(TweakGrammar::Type::Vector2): return ETweakFlatType::Vector2;
    case CName(TweakGrammar::Type::Color): return ETweakFlatType::Color;
    }

    return ETweakFlatType::Undefined;
}

Red::ETweakFlatOp Red::TweakParser::ResolveOperation(const std::string& aInput)
{
    switch (CName(aInput.c_str()))
    {
    case CName(TweakGrammar::Op::Assign): return ETweakFlatOp::Assign;
    case CName(TweakGrammar::Op::Append): return ETweakFlatOp::Append;
    case CName(TweakGrammar::Op::Remove): return ETweakFlatOp::Remove;
    }

    return Red::ETweakFlatOp::Undefined;
}

std::string Red::TweakParser::FormatError(const std::filesystem::path& aPath,
                                          const tao::pegtl::position& aPosition,
                                          const std::string_view& aMessage)
{
    return std::format("{}:{}:{}: {}", aPath.filename().string(), aPosition.line, aPosition.column, aMessage);
}

Core::SharedPtr<Red::TweakSource> Red::TweakParser::Parse(const std::filesystem::path& aPath)
{
    TweakSource package;
    ParseState state;
    tao::pegtl::file_input input(aPath);

    try
    {
        bool success = tao::pegtl::parse<TweakGrammar::source, ParseAction, ParseControl>(input, state, package);

        if (!success || !input.empty())
        {
            throw std::exception(FormatError(aPath, input.position(), "Unexpected end of file").c_str());
        }
    }
    catch (const tao::pegtl::parse_error& e)
    {
        const auto& position = e.positions().front();
        const auto& message = e.message();

        throw std::exception(FormatError(aPath, position, message).c_str());
    }

    return Core::MakeShared<TweakSource>(std::move(package));
}
