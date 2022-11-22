#include "YamlConverter.hpp"
#include "App/Utils/Str.hpp"
#include "Red/Localization.hpp"
#include "Red/Rtti/Utils.hpp"
#include "Red/TweakDB/Reflection.hpp"

template<typename T>
Core::SharedPtr<T> App::YamlConverter::Convert(const YAML::Node& aNode, bool)
{
    auto value = Core::MakeShared<T>();
    if (YAML::convert<T>::decode(aNode, *value))
        return value;

    return nullptr;
}

template<>
Core::SharedPtr<Red::CName> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    // Quoted format: n"Name"
    constexpr const char* QuotedPrefix = "n\"";
    constexpr const char QuotedSuffix = '"';
    constexpr size_t QuotedSkip = std::char_traits<char>::length(QuotedPrefix);
    constexpr size_t QuotedDiff = QuotedSkip + sizeof(QuotedSuffix);

    // Wrapped format: CName("Name")
    constexpr const char* WrappedPrefix = "CName(\"";
    constexpr const char* WrappedSuffix = "\")";
    constexpr size_t WrappedSkip = std::char_traits<char>::length(WrappedPrefix);
    constexpr size_t WrappedDiff = WrappedSkip + std::char_traits<char>::length(WrappedSuffix);

    if (aNode.IsScalar())
    {
        if (aStrict && aNode.Tag() == "!")
            return nullptr;

        const auto& str = aNode.Scalar();

        if (str.starts_with(QuotedPrefix) && str.ends_with(QuotedSuffix))
        {
            return Core::MakeShared<Red::CName>(
                Red::CNamePool::Add(str.substr(QuotedSkip, str.length() - QuotedDiff).c_str()));
        }

        if (str.starts_with(WrappedPrefix) && str.ends_with(WrappedSuffix))
        {
            return Core::MakeShared<Red::CName>(
                Red::CNamePool::Add(str.substr(WrappedSkip, str.length() - WrappedDiff).c_str()));
        }

        if (!aStrict)
        {
            return Core::MakeShared<Red::CName>(Red::CNamePool::Add(str.c_str()));
        }
    }

    return nullptr;
}

template<>
Core::SharedPtr<Red::TweakDBID> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    // Quoted format: t"Package.Item"
    constexpr const char* QuotedPrefix = "t\"";
    constexpr const char QuotedSuffix = '"';
    constexpr size_t QuotedSkip = std::char_traits<char>::length(QuotedPrefix);
    constexpr size_t QuotedDiff = QuotedSkip + sizeof(QuotedSuffix);

    // Wrapped format: TweakDBID("Package.Item")
    constexpr const char* WrappedPrefix = "TweakDBID(\"";
    constexpr const char* WrappedSuffix = "\")";
    constexpr size_t WrappedSkip = std::char_traits<char>::length(WrappedPrefix);
    constexpr size_t WrappedDiff = WrappedSkip + std::char_traits<char>::length(WrappedSuffix);

    // Debug format: <TDBID:12345678:12>
    constexpr const char* DebugPrefix = "<TDBID:";
    constexpr const char DebugSuffix = '>';
    constexpr size_t DebugLength = std::char_traits<char>::length("<TDBID:12345678:12>");
    constexpr size_t DebugHashPos = std::char_traits<char>::length(DebugPrefix);
    constexpr size_t DebugHashSize = 8;
    constexpr size_t DebugLenPos = DebugHashPos + DebugHashSize + sizeof(DebugSuffix);
    constexpr size_t DebugLenSize = 2;
    static_assert(DebugLength == DebugHashPos + DebugHashSize + 1 + DebugLenSize + sizeof(DebugSuffix));

    // Special values
    constexpr const char* EmptyValue = "None";

    if (aNode.IsScalar())
    {
        if (aStrict && aNode.Tag() == "!")
            return nullptr;

        const auto& str = aNode.Scalar();

        if (str.starts_with(QuotedPrefix) && str.ends_with(QuotedSuffix))
        {
            return Core::MakeShared<Red::TweakDBID>(
                str.substr(QuotedSkip, str.length() - QuotedDiff));
        }

        if (str.starts_with(WrappedPrefix) && str.ends_with(WrappedSuffix))
        {
            return Core::MakeShared<Red::TweakDBID>(
                str.substr(WrappedSkip, str.length() - WrappedDiff));
        }

        if (str.length() == DebugLength && str.starts_with(DebugPrefix) && str.ends_with(DebugSuffix))
        {
            auto hash = ParseInt<uint32_t>(str.substr(DebugHashPos, DebugHashSize), 16);
            auto len = ParseInt<uint8_t>(str.substr(DebugLenPos, DebugLenSize), 16);

            return Core::MakeShared<Red::TweakDBID>(hash, len);
        }

        if (!aStrict)
        {
            if (str == EmptyValue)
                return Core::MakeShared<Red::TweakDBID>();

            return Core::MakeShared<Red::TweakDBID>(str);
        }
    }

    return nullptr;
}

template<>
Core::SharedPtr<Red::LocKeyWrapper> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    // Quoted format: l"Secondary-Loc-Key"
    constexpr const char* QuotedPrefix = "l\"";
    constexpr const char QuotedSuffix = '"';
    constexpr size_t QuotedSkip = std::char_traits<char>::length(QuotedPrefix);
    constexpr size_t QuotedDiff = QuotedSkip + sizeof(QuotedSuffix);

    // Wrapped format: LocKey("Secondary-Loc-Key") | LocKey(12345)
    constexpr const char* WrappedPrefix = "LocKey(";
    constexpr const char WrappedSuffix = ')';
    constexpr size_t WrappedSkip = std::char_traits<char>::length(WrappedPrefix);
    constexpr size_t WrappedDiff = WrappedSkip + sizeof(WrappedSuffix);

    // String format: LocKey#Secondary-Loc-Key | LocKey#12345
    constexpr const char* StringPrefix = Red::LocKeyPrefix;
    constexpr size_t StringSkip = std::char_traits<char>::length(StringPrefix);
    constexpr size_t StringDiff = StringSkip;

    if (aNode.IsScalar())
    {
        if (aStrict && aNode.Tag() == "!")
            return nullptr;

        const auto& str = aNode.Scalar();

        if (str.starts_with(QuotedPrefix) && str.ends_with(QuotedSuffix))
        {
            return Core::MakeShared<Red::LocKeyWrapper>(
                str.substr(QuotedSkip, str.length() - QuotedDiff).c_str());
        }

        if (str.starts_with(WrappedPrefix) && str.ends_with(WrappedSuffix))
        {
            const size_t quoted = str[WrappedSkip] == '"' ? 1 : 0;
            auto value = str.substr(WrappedSkip + quoted, str.length() - WrappedDiff - (quoted << 1));

            if (quoted)
                return Core::MakeShared<Red::LocKeyWrapper>(value.c_str());

            uint64_t key;
            if (ParseInt(value, key))
                return Core::MakeShared<Red::LocKeyWrapper>(key);

            return nullptr;
        }

        if (str.starts_with(StringPrefix))
        {
            auto value = str.substr(StringSkip, str.length() - StringDiff);

            uint64_t key;
            if (ParseInt(value, key))
                return Core::MakeShared<Red::LocKeyWrapper>(key);

            return Core::MakeShared<Red::LocKeyWrapper>(value.c_str());
        }

        if (!aStrict)
        {
            uint64_t key;
            if (ParseInt(str, key))
                return Core::MakeShared<Red::LocKeyWrapper>(key);

            return Core::MakeShared<Red::LocKeyWrapper>(str.c_str());
        }
    }

    return nullptr;
}

template<>
Core::SharedPtr<Red::ResourceAsyncReference<>> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    // Quoted format: r"base\gameplay\resource.ext"
    constexpr const char* QuotedPrefix = "r\"";
    constexpr const char QuotedSuffix = '"';
    constexpr size_t QuotedSkip = std::char_traits<char>::length(QuotedPrefix);
    constexpr size_t QuotedDiff = QuotedSkip + sizeof(QuotedSuffix);

    // Wrapped format: ResRef("base\gameplay\resource.ext") | ResRef(123456789)
    constexpr const char* WrappedPrefix = "ResRef(";
    constexpr const char WrappedSuffix = ')';
    constexpr size_t WrappedSkip = std::char_traits<char>::length(WrappedPrefix);
    constexpr size_t WrappedDiff = WrappedSkip + sizeof(WrappedSuffix);

    if (aNode.IsScalar())
    {
        if (aStrict && aNode.Tag() == "!")
            return nullptr;

        const auto& str = aNode.Scalar();

        if (str.starts_with(QuotedPrefix) && str.ends_with(QuotedSuffix))
        {
            auto value = str.substr(QuotedSkip, str.length() - QuotedDiff);

            return Core::MakeShared<Red::ResourceAsyncReference<>>(value.c_str());
        }

        if (str.starts_with(WrappedPrefix) && str.ends_with(WrappedSuffix))
        {
            const size_t quoted = str[WrappedSkip] == '"' ? 1 : 0;
            auto value = str.substr(WrappedSkip + quoted, str.length() - WrappedDiff - (quoted << 1));

            if (quoted)
                return Core::MakeShared<Red::ResourceAsyncReference<>>(value.c_str());

            uint64_t hash;
            if (ParseInt(value, hash))
                return Core::MakeShared<Red::ResourceAsyncReference<>>(hash);

            return nullptr;
        }

        if (!aStrict)
        {
            uint64_t hash;
            if (ParseInt(str, hash))
                return Core::MakeShared<Red::ResourceAsyncReference<>>(hash);

            return Core::MakeShared<Red::ResourceAsyncReference<>>(str.c_str());
        }
    }

    return nullptr;
}

template<>
Core::SharedPtr<Red::CString> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (!aNode.IsScalar())
        return nullptr;

    if (aStrict)
    {
        // In strict mode we accept only quoted strings.
        // According to the spec the parser must set the tag to "!" for quoted strings.
        if (aNode.Tag() != "!")
            return nullptr;
    }
    else
    {
        const auto locKey = Convert<Red::LocKeyWrapper>(aNode, true);
        if (locKey)
        {
            const auto locKeyStr = std::string(Red::LocKeyPrefix).append(std::to_string(locKey->primaryKey));

            return Core::MakeShared<Red::CString>(locKeyStr.c_str());
        }
    }

    return Core::MakeShared<Red::CString>(aNode.Scalar().c_str());
}

template<>
Core::SharedPtr<Red::Quaternion> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["i"] || !aNode["j"] || !aNode["k"] || !aNode["r"]))
            return nullptr;

        auto value = Core::MakeShared<Red::Quaternion>();
        value->i = aNode["i"].as<float>(0.0f);
        value->j = aNode["j"].as<float>(0.0f);
        value->k = aNode["k"].as<float>(0.0f);
        value->r = aNode["r"].as<float>(0.0f);

        return value;
    }

    return nullptr;
}

template<>
Core::SharedPtr<Red::EulerAngles> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["roll"] || !aNode["pitch"] || !aNode["yaw"]))
            return nullptr;

        auto value = Core::MakeShared<Red::EulerAngles>();
        value->Roll = aNode["roll"].as<float>(0.0f);
        value->Pitch = aNode["pitch"].as<float>(0.0f);
        value->Yaw = aNode["yaw"].as<float>(0.0f);

        return value;
    }

    return nullptr;
}

template<>
Core::SharedPtr<Red::Vector3> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["x"] || !aNode["y"] || !aNode["z"]))
            return nullptr;

        auto value = Core::MakeShared<Red::Vector3>();
        value->X = aNode["x"].as<float>(0.0f);
        value->Y = aNode["y"].as<float>(0.0f);
        value->Z = aNode["z"].as<float>(0.0f);

        return value;
    }

    return nullptr;
}

template<>
Core::SharedPtr<Red::Vector2> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["x"] || !aNode["y"]))
            return nullptr;

        auto value = Core::MakeShared<Red::Vector2>();
        value->X = aNode["x"].as<float>(0.0f);
        value->Y = aNode["y"].as<float>(0.0f);

        return value;
    }

    return nullptr;
}

template<>
Core::SharedPtr<Red::Color> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["red"] || !aNode["green"] || !aNode["blue"] || !aNode["alpha"]))
            return nullptr;

        auto value = Core::MakeShared<Red::Color>();
        value->Red = aNode["red"].as<uint8_t>(0);
        value->Green = aNode["green"].as<uint8_t>(0);
        value->Blue = aNode["blue"].as<uint8_t>(0);
        value->Alpha = aNode["alpha"].as<uint8_t>(0);

        return value;
    }

    return nullptr;
}

template<typename E>
Core::SharedPtr<Red::DynArray<E>> App::YamlConverter::ConvertArray(const YAML::Node& aNode, bool)
{
    if (aNode.IsSequence())
    {
        auto array = Core::MakeShared<Red::DynArray<E>>();

        if (aNode.size() > 0)
        {
            array->Reserve(static_cast<uint32_t>(aNode.size()));

            for (const auto& item : aNode)
            {
                const auto value = Convert<E>(item);

                // Abort on first incompatible element
                if (!value)
                    return nullptr;

                array->PushBack(*value);
            }
        }

        return array;
    }

    return nullptr;
}

template<typename T>
bool App::YamlConverter::Convert(const YAML::Node& aNode, Core::SharedPtr<void>& aValue, bool aStrict)
{
    aValue = Convert<T>(aNode, aStrict);
    return aValue != nullptr;
}

template<typename E>
bool App::YamlConverter::ConvertArray(const YAML::Node& aNode, Core::SharedPtr<void>& aValue, bool aStrict)
{
    aValue = ConvertArray<E>(aNode, aStrict);
    return aValue != nullptr;
}

Core::SharedPtr<void> App::YamlConverter::Convert(const Red::CBaseRTTIType* aType, const YAML::Node& aNode)
{
    return Convert(aType->GetName(), aNode);
}

Core::SharedPtr<void> App::YamlConverter::Convert(Red::CName aTypeName, const YAML::Node& aNode)
{
    switch (aTypeName)
    {
    case Red::ERTDBFlatType::Int:
        return Convert<int>(aNode);
    case Red::ERTDBFlatType::IntArray:
        return ConvertArray<int>(aNode);

    case Red::ERTDBFlatType::Float:
        return Convert<float>(aNode);
    case Red::ERTDBFlatType::FloatArray:
        return ConvertArray<float>(aNode);

    case Red::ERTDBFlatType::Bool:
        return Convert<bool>(aNode);
    case Red::ERTDBFlatType::BoolArray:
        return ConvertArray<bool>(aNode);

    case Red::ERTDBFlatType::String:
        return Convert<Red::CString>(aNode);
    case Red::ERTDBFlatType::StringArray:
        return ConvertArray<Red::CString>(aNode);

    case Red::ERTDBFlatType::CName:
        return Convert<Red::CName>(aNode);
    case Red::ERTDBFlatType::CNameArray:
        return ConvertArray<Red::CName>(aNode);

    case Red::ERTDBFlatType::TweakDBID:
        return Convert<Red::TweakDBID>(aNode);
    case Red::ERTDBFlatType::TweakDBIDArray:
        return ConvertArray<Red::TweakDBID>(aNode);

    case Red::ERTDBFlatType::LocKey:
        return Convert<Red::LocKeyWrapper>(aNode);
    case Red::ERTDBFlatType::LocKeyArray:
        return ConvertArray<Red::LocKeyWrapper>(aNode);

    case Red::ERTDBFlatType::ResRef:
        return Convert<Red::ResourceAsyncReference<>>(aNode);
    case Red::ERTDBFlatType::ResRefArray:
        return ConvertArray<Red::ResourceAsyncReference<>>(aNode);

    case Red::ERTDBFlatType::Quaternion:
        return Convert<Red::Quaternion>(aNode);
    case Red::ERTDBFlatType::QuaternionArray:
        return ConvertArray<Red::Quaternion>(aNode);

    case Red::ERTDBFlatType::EulerAngles:
        return Convert<Red::EulerAngles>(aNode);
    case Red::ERTDBFlatType::EulerAnglesArray:
        return ConvertArray<Red::EulerAngles>(aNode);

    case Red::ERTDBFlatType::Vector3:
        return Convert<Red::Vector3>(aNode);
    case Red::ERTDBFlatType::Vector3Array:
        return ConvertArray<Red::Vector3>(aNode);

    case Red::ERTDBFlatType::Vector2:
        return Convert<Red::Vector2>(aNode);
    case Red::ERTDBFlatType::Vector2Array:
        return ConvertArray<Red::Vector2>(aNode);

    case Red::ERTDBFlatType::Color:
        return Convert<Red::Color>(aNode);
    case Red::ERTDBFlatType::ColorArray:
        return ConvertArray<Red::Color>(aNode);
    }

    return nullptr;
}

std::pair<Red::CName, Core::SharedPtr<void>> App::YamlConverter::Convert(const YAML::Node& aNode)
{
    Core::SharedPtr<void> value;

    switch (aNode.Type())
    {
    case YAML::NodeType::Scalar:
    {
        if (Convert<Red::CString>(aNode, value, true))
            return { Red::ERTDBFlatType::String, value };

        if (Convert<int>(aNode, value, true))
            return { Red::ERTDBFlatType::Int, value };

        if (Convert<float>(aNode, value, true))
            return { Red::ERTDBFlatType::Float, value };

        if (Convert<bool>(aNode, value, true))
            return { Red::ERTDBFlatType::Bool, value };

        if (Convert<Red::CName>(aNode, value, true))
            return { Red::ERTDBFlatType::CName, value };

        if (Convert<Red::TweakDBID>(aNode, value, true))
            return { Red::ERTDBFlatType::TweakDBID, value };

        if (Convert<Red::LocKeyWrapper>(aNode, value, true))
            return { Red::ERTDBFlatType::LocKey, value };

        if (Convert<Red::ResourceAsyncReference<>>(aNode, value, true))
            return { Red::ERTDBFlatType::ResRef, value };

        break;
    }
    case YAML::NodeType::Map:
    {
        if (Convert<Red::Quaternion>(aNode, value, true))
            return { Red::ERTDBFlatType::Quaternion, value };

        if (Convert<Red::EulerAngles>(aNode, value, true))
            return { Red::ERTDBFlatType::EulerAngles, value };

        if (Convert<Red::Vector3>(aNode, value, true))
            return { Red::ERTDBFlatType::Vector3, value };

        if (Convert<Red::Vector2>(aNode, value, true))
            return { Red::ERTDBFlatType::Vector2, value };

        if (Convert<Red::Color>(aNode, value, true))
            return { Red::ERTDBFlatType::Color, value };

        break;
    }
    case YAML::NodeType::Sequence:
    {
        // Try to resolve array type from first element...
        if (aNode.size() > 0)
        {
            const auto item = Convert(aNode[0]);

            if (item.second)
            {
                auto arrayTypeName = Red::ToArrayType(item.first);
                return { arrayTypeName, Convert(arrayTypeName, aNode) };
            }
        }
        break;
    }
    }

    return { {}, value };
}
