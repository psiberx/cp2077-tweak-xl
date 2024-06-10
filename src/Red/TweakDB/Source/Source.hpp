#pragma once

namespace Red
{
struct TweakGroup;
struct TweakFlat;
struct TweakValue;
struct TweakInline;

enum class ETweakValueType
{
    Undefined,
    Bool,
    Number,
    String,
    Struct,
    Inline,
};

enum class ETweakFlatType
{
    Undefined,
    Int,
    Float,
    Bool,
    String,
    CName,
    LocKey,
    ResRef,
    Quaternion,
    EulerAngles,
    Vector3,
    Vector2,
    Color,
    ForeignKey,
};

enum class ETweakFlatOp
{
    Undefined,
    Assign,
    Append,
    Remove,
};

struct TweakValue
{
    ETweakValueType type;
    Core::Vector<std::string> data;
    Core::SharedPtr<TweakGroup> group;
};

struct TweakFlat
{
    std::string name;
    ETweakFlatType type;
    std::string foreignType;
    bool isArray;
    ETweakFlatOp operation;
    Core::Vector<Core::SharedPtr<TweakValue>> values;
    Core::Vector<std::string> tags;
};

struct TweakGroup
{
    std::string name;
    std::string base;
    Core::Vector<Core::SharedPtr<TweakFlat>> flats;
    Core::Vector<Core::SharedPtr<TweakInline>> inlines;
    Core::Vector<std::string> tags;
    bool isSchema;
    bool isQuery;
};

struct TweakInline
{
    Core::SharedPtr<TweakGroup> group;
    Core::SharedPtr<TweakGroup> owner;
    Core::SharedPtr<TweakGroup> parent;
};

struct TweakSource
{
    static constexpr auto Extension = L".tweak";
    static constexpr auto SchemaPackage = "RTDB";
    static constexpr auto QueryPackage = "Query";

    std::string package;
    Core::Vector<std::string> usings;
    Core::Vector<Core::SharedPtr<TweakGroup>> groups;
    Core::Vector<Core::SharedPtr<TweakFlat>> flats;
    Core::Vector<Core::SharedPtr<TweakInline>> inlines;
    bool isPackage;
    bool isSchema;
    bool isQuery;
};

using TweakGroupPtr = Core::SharedPtr<TweakGroup>;
using TweakFlatPtr = Core::SharedPtr<TweakFlat>;
using TweakValuePtr = Core::SharedPtr<TweakValue>;
using TweakInlinePtr = Core::SharedPtr<TweakInline>;
using TweakSourcePtr = Core::SharedPtr<TweakSource>;
}
