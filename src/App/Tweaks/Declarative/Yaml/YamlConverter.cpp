#include "YamlConverter.hpp"
#include "App/Utils/Str.hpp"
#include "Red/TweakDB/Types.hpp"

template<typename T>
Core::SharedPtr<T> App::YamlConverter::Convert(const YAML::Node& aNode, bool)
{
    auto value = Core::MakeShared<T>();
    if (YAML::convert<T>::decode(aNode, *value))
        return value;

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::CString> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (!aNode.IsScalar())
        return nullptr;

    // In strict mode accept only quoted strings.
    // According to the spec the parser must set the tag to "!" for quoted strings.
    if (aStrict && aNode.Tag() != "!")
        return nullptr;

    return Core::MakeShared<RED4ext::CString>(aNode.Scalar().c_str());
}

template<>
Core::SharedPtr<RED4ext::CName> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
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
            return Core::MakeShared<RED4ext::CName>(
                RED4ext::CNamePool::Add(str.substr(QuotedSkip, str.length() - QuotedDiff).c_str()));
        }

        if (str.starts_with(WrappedPrefix) && str.ends_with(WrappedSuffix))
        {
            return Core::MakeShared<RED4ext::CName>(
                RED4ext::CNamePool::Add(str.substr(WrappedSkip, str.length() - WrappedDiff).c_str()));
        }

        if (!aStrict)
        {
            return Core::MakeShared<RED4ext::CName>(RED4ext::CNamePool::Add(str.c_str()));
        }
    }

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::TweakDBID> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
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

    if (aNode.IsScalar())
    {
        if (aStrict && aNode.Tag() == "!")
            return nullptr;

        const auto& str = aNode.Scalar();

        if (str.starts_with(QuotedPrefix) && str.ends_with(QuotedSuffix))
        {
            return Core::MakeShared<RED4ext::TweakDBID>(
                str.substr(QuotedSkip, str.length() - QuotedDiff));
        }

        if (str.starts_with(WrappedPrefix) && str.ends_with(WrappedSuffix))
        {
            return Core::MakeShared<RED4ext::TweakDBID>(
                str.substr(WrappedSkip, str.length() - WrappedDiff));
        }

        if (str.length() == DebugLength && str.starts_with(DebugPrefix) && str.ends_with(DebugSuffix))
        {
            auto hash = ParseInt<uint32_t>(str.substr(DebugHashPos, DebugHashSize), 16);
            auto len = ParseInt<uint8_t>(str.substr(DebugLenPos, DebugLenSize), 16);

            return Core::MakeShared<RED4ext::TweakDBID>(hash, len);
        }

        if (!aStrict)
        {
            return Core::MakeShared<RED4ext::TweakDBID>(str);
        }
    }

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::gamedataLocKeyWrapper> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
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

    if (aNode.IsScalar())
    {
        if (aStrict && aNode.Tag() == "!")
            return nullptr;

        const auto& str = aNode.Scalar();

        if (str.starts_with(QuotedPrefix) && str.ends_with(QuotedSuffix))
        {
            return Core::MakeShared<RED4ext::gamedataLocKeyWrapper>(
                str.substr(QuotedSkip, str.length() - QuotedDiff).c_str());
        }

        if (str.starts_with(WrappedPrefix) && str.ends_with(WrappedSuffix))
        {
            const size_t quoted = str[WrappedSkip] == '"' ? 1 : 0;
            auto value = str.substr(WrappedSkip + quoted, str.length() - WrappedDiff - (quoted << 1));

            if (quoted)
                return Core::MakeShared<RED4ext::gamedataLocKeyWrapper>(value.c_str());

            uint64_t key;
            if (ParseInt(value, key))
                return Core::MakeShared<RED4ext::gamedataLocKeyWrapper>(key);

            return nullptr;
        }

        if (!aStrict)
        {
            uint64_t key;
            if (ParseInt(str, key))
                return Core::MakeShared<RED4ext::gamedataLocKeyWrapper>(key);

            return Core::MakeShared<RED4ext::gamedataLocKeyWrapper>(str.c_str());
        }
    }

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::ResourceAsyncReference<>> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
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

            return Core::MakeShared<RED4ext::ResourceAsyncReference<>>(value.c_str());
        }

        if (str.starts_with(WrappedPrefix) && str.ends_with(WrappedSuffix))
        {
            const size_t quoted = str[WrappedSkip] == '"' ? 1 : 0;
            auto value = str.substr(WrappedSkip + quoted, str.length() - WrappedDiff - (quoted << 1));

            if (quoted)
                return Core::MakeShared<RED4ext::ResourceAsyncReference<>>(value.c_str());

            uint64_t hash;
            if (ParseInt(value, hash))
                return Core::MakeShared<RED4ext::ResourceAsyncReference<>>(hash);

            return nullptr;
        }

        if (!aStrict)
        {
            uint64_t hash;
            if (ParseInt(str, hash))
                return Core::MakeShared<RED4ext::ResourceAsyncReference<>>(hash);

            return Core::MakeShared<RED4ext::ResourceAsyncReference<>>(str.c_str());
        }
    }

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::Quaternion> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["i"] || !aNode["j"] || !aNode["k"] || !aNode["r"]))
            return nullptr;

        auto value = Core::MakeShared<RED4ext::Quaternion>();
        value->i = aNode["i"].as<float>(0.0f);
        value->j = aNode["j"].as<float>(0.0f);
        value->k = aNode["k"].as<float>(0.0f);
        value->r = aNode["r"].as<float>(0.0f);

        return value;
    }

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::EulerAngles> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["roll"] || !aNode["pitch"] || !aNode["yaw"]))
            return nullptr;

        auto value = Core::MakeShared<RED4ext::EulerAngles>();
        value->Roll = aNode["roll"].as<float>(0.0f);
        value->Pitch = aNode["pitch"].as<float>(0.0f);
        value->Yaw = aNode["yaw"].as<float>(0.0f);

        return value;
    }

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::Vector3> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["x"] || !aNode["y"] || !aNode["z"]))
            return nullptr;

        auto value = Core::MakeShared<RED4ext::Vector3>();
        value->X = aNode["x"].as<float>(0.0f);
        value->Y = aNode["y"].as<float>(0.0f);
        value->Z = aNode["z"].as<float>(0.0f);

        return value;
    }

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::Vector2> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["x"] || !aNode["y"]))
            return nullptr;

        auto value = Core::MakeShared<RED4ext::Vector2>();
        value->X = aNode["x"].as<float>(0.0f);
        value->Y = aNode["y"].as<float>(0.0f);

        return value;
    }

    return nullptr;
}

template<>
Core::SharedPtr<RED4ext::Color> App::YamlConverter::Convert(const YAML::Node& aNode, bool aStrict)
{
    if (aNode.IsMap())
    {
        if (aStrict && (!aNode["red"] || !aNode["green"] || !aNode["blue"] || !aNode["alpha"]))
            return nullptr;

        auto value = Core::MakeShared<RED4ext::Color>();
        value->Red = aNode["red"].as<uint8_t>(0);
        value->Green = aNode["green"].as<uint8_t>(0);
        value->Blue = aNode["blue"].as<uint8_t>(0);
        value->Alpha = aNode["alpha"].as<uint8_t>(0);

        return value;
    }

    return nullptr;
}

template<typename E>
Core::SharedPtr<RED4ext::DynArray<E>> App::YamlConverter::ConvertArray(const YAML::Node& aNode, bool)
{
    if (aNode.IsSequence())
    {
        auto array = Core::MakeShared<RED4ext::DynArray<E>>();

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

Core::SharedPtr<void> App::YamlConverter::Convert(const RED4ext::CBaseRTTIType* aType, const YAML::Node& aNode)
{
    return Convert(aType->GetName(), aNode);
}

Core::SharedPtr<void> App::YamlConverter::Convert(RED4ext::CName aTypeName, const YAML::Node& aNode)
{
    switch (aTypeName)
    {
    case Red::TweakDB::EFlatType::Int32:
        return Convert<int>(aNode);
    case Red::TweakDB::EFlatType::Int32Array:
        return ConvertArray<int>(aNode);

    case Red::TweakDB::EFlatType::Float:
        return Convert<float>(aNode);
    case Red::TweakDB::EFlatType::FloatArray:
        return ConvertArray<float>(aNode);

    case Red::TweakDB::EFlatType::Bool:
        return Convert<bool>(aNode);
    case Red::TweakDB::EFlatType::BoolArray:
        return ConvertArray<bool>(aNode);

    case Red::TweakDB::EFlatType::String:
        return Convert<RED4ext::CString>(aNode);
    case Red::TweakDB::EFlatType::StringArray:
        return ConvertArray<RED4ext::CString>(aNode);

    case Red::TweakDB::EFlatType::CName:
        return Convert<RED4ext::CName>(aNode);
    case Red::TweakDB::EFlatType::CNameArray:
        return ConvertArray<RED4ext::CName>(aNode);

    case Red::TweakDB::EFlatType::TweakDBID:
        return Convert<RED4ext::TweakDBID>(aNode);
    case Red::TweakDB::EFlatType::TweakDBIDArray:
        return ConvertArray<RED4ext::TweakDBID>(aNode);

    case Red::TweakDB::EFlatType::LocKey:
        return Convert<RED4ext::gamedataLocKeyWrapper>(aNode);
    case Red::TweakDB::EFlatType::LocKeyArray:
        return ConvertArray<RED4ext::gamedataLocKeyWrapper>(aNode);

    case Red::TweakDB::EFlatType::Resource:
        return Convert<RED4ext::ResourceAsyncReference<>>(aNode);
    case Red::TweakDB::EFlatType::ResourceArray:
        return ConvertArray<RED4ext::ResourceAsyncReference<>>(aNode);

    case Red::TweakDB::EFlatType::Quaternion:
        return Convert<RED4ext::Quaternion>(aNode);
    case Red::TweakDB::EFlatType::QuaternionArray:
        return ConvertArray<RED4ext::Quaternion>(aNode);

    case Red::TweakDB::EFlatType::EulerAngles:
        return Convert<RED4ext::EulerAngles>(aNode);
    case Red::TweakDB::EFlatType::EulerAnglesArray:
        return ConvertArray<RED4ext::EulerAngles>(aNode);

    case Red::TweakDB::EFlatType::Vector3:
        return Convert<RED4ext::Vector3>(aNode);
    case Red::TweakDB::EFlatType::Vector3Array:
        return ConvertArray<RED4ext::Vector3>(aNode);

    case Red::TweakDB::EFlatType::Vector2:
        return Convert<RED4ext::Vector2>(aNode);
    case Red::TweakDB::EFlatType::Vector2Array:
        return ConvertArray<RED4ext::Vector2>(aNode);

    case Red::TweakDB::EFlatType::Color:
        return Convert<RED4ext::Color>(aNode);
    case Red::TweakDB::EFlatType::ColorArray:
        return ConvertArray<RED4ext::Color>(aNode);
    }

    return nullptr;
}

std::pair<RED4ext::CName, Core::SharedPtr<void>> App::YamlConverter::Convert(const YAML::Node& aNode)
{
    Core::SharedPtr<void> value;

    switch (aNode.Type())
    {
    case YAML::NodeType::Scalar:
    {
        if (Convert<RED4ext::CString>(aNode, value, true))
            return { Red::TweakDB::EFlatType::String, value };

        if (Convert<int>(aNode, value, true))
            return { Red::TweakDB::EFlatType::Int32, value };

        if (Convert<float>(aNode, value, true))
            return { Red::TweakDB::EFlatType::Float, value };

        if (Convert<bool>(aNode, value, true))
            return { Red::TweakDB::EFlatType::Bool, value };

        if (Convert<RED4ext::CName>(aNode, value, true))
            return { Red::TweakDB::EFlatType::CName, value };

        if (Convert<RED4ext::TweakDBID>(aNode, value, true))
            return { Red::TweakDB::EFlatType::TweakDBID, value };

        if (Convert<RED4ext::gamedataLocKeyWrapper>(aNode, value, true))
            return { Red::TweakDB::EFlatType::LocKey, value };

        if (Convert<RED4ext::ResourceAsyncReference<>>(aNode, value, true))
            return { Red::TweakDB::EFlatType::Resource, value };

        break;
    }
    case YAML::NodeType::Map:
    {
        if (Convert<RED4ext::Quaternion>(aNode, value, true))
            return { Red::TweakDB::EFlatType::Quaternion, value };

        if (Convert<RED4ext::EulerAngles>(aNode, value, true))
            return { Red::TweakDB::EFlatType::EulerAngles, value };

        if (Convert<RED4ext::Vector3>(aNode, value, true))
            return { Red::TweakDB::EFlatType::Vector3, value };

        if (Convert<RED4ext::Vector2>(aNode, value, true))
            return { Red::TweakDB::EFlatType::Vector2, value };

        if (Convert<RED4ext::Color>(aNode, value, true))
            return { Red::TweakDB::EFlatType::Color, value };

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
                const auto arrayTypeName = Red::TweakDB::GetArrayType(item.first);
                return { arrayTypeName, Convert(arrayTypeName, aNode) };
            }
        }

        break;
    }
    }

    return { {}, value };
}
