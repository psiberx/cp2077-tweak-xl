#include "RedReader.hpp"
#include "App/Utils/Str.hpp"
#include "Red/Localization.hpp"
#include "Red/TweakDB/Source/Grammar.hpp"

namespace
{
template<typename T>
requires std::is_integral_v<T>
inline bool ParseInt(const std::string& aData, T& aResult)
{
    return App::ParseInt(aData, aResult);
}

inline bool ParseFloat(const std::string& aData, float& aResult)
{
    return App::ParseFloat(aData, aResult, Red::TweakGrammar::Float::Suffix);
}

template<typename T>
Core::SharedPtr<T> ConvertValue(const Red::TweakValuePtr& aValue);

template<>
Core::SharedPtr<int> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::Number)
    {
        auto& data = aValue->data.front();
        int result;

        if (ParseInt(data, result))
        {
            return Core::MakeShared<int>(result);
        }
    }

    return {};
}

template<>
Core::SharedPtr<float> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::Number)
    {
        auto& data = aValue->data.front();
        float result;

        if (ParseFloat(data, result))
        {
            return Core::MakeShared<float>(result);
        }
    }

    return {};
}

template<>
Core::SharedPtr<bool> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::Bool)
    {
        auto& data = aValue->data.front();

        return Core::MakeShared<bool>(data == Red::TweakGrammar::Bool::True);
    }

    return {};
}

template<>
Core::SharedPtr<Red::LocKeyWrapper> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::String)
    {
        auto& data = aValue->data.front();

        if (data.empty())
        {
            return Core::MakeShared<Red::LocKeyWrapper>();
        }

        if (data.starts_with(Red::LocKeyPrefix))
        {
            const auto key = data.substr(Red::LocKeyPrefixLength);

            uint64_t hash;
            if (ParseInt(key, hash))
            {
                return Core::MakeShared<Red::LocKeyWrapper>(hash);
            }

            return Core::MakeShared<Red::LocKeyWrapper>(key.c_str());
        }
    }

    return {};
}

template<>
Core::SharedPtr<Red::CString> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::String)
    {
        auto& data = aValue->data.front();

        if (data.empty())
        {
            return Core::MakeShared<Red::CString>();
        }

        if (const auto locKey = ConvertValue<Red::LocKeyWrapper>(aValue))
        {
            return Core::MakeShared<Red::CString>(
                std::string(Red::LocKeyPrefix).append(std::to_string(locKey->primaryKey)).c_str());
        }

        return Core::MakeShared<Red::CString>(data.c_str());
    }

    return {};
}

template<>
Core::SharedPtr<Red::CName> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::String)
    {
        auto& data = aValue->data.front();

        if (data.empty())
        {
            return Core::MakeShared<Red::CName>();
        }

        return Core::MakeShared<Red::CName>(data.c_str());
    }

    return {};
}

template<>
Core::SharedPtr<Red::ResourceAsyncReference<>> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::String)
    {
        auto& data = aValue->data.front();

        return Core::MakeShared<Red::ResourceAsyncReference<>>(data.c_str());
    }

    return {};
}

template<>
Core::SharedPtr<Red::TweakDBID> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::String)
    {
        auto& data = aValue->data.front();

        if (data.empty())
        {
            return Core::MakeShared<Red::TweakDBID>();
        }

        return Core::MakeShared<Red::TweakDBID>(data.c_str());
    }

    return {};
}

template<>
Core::SharedPtr<Red::Quaternion> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::Struct && aValue->data.size() == 4)
    {
        auto& data = aValue->data;
        auto result = Core::MakeShared<Red::Quaternion>();

        if (ParseFloat(data[0], result->i) && ParseFloat(data[1], result->j)
            && ParseFloat(data[2], result->k) && ParseFloat(data[3], result->r))
        {
            return result;
        }
    }

    return {};
}

template<>
Core::SharedPtr<Red::EulerAngles> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::Struct && aValue->data.size() == 3)
    {
        auto& data = aValue->data;
        auto result = Core::MakeShared<Red::EulerAngles>();

        if (ParseFloat(data[0], result->Roll) && ParseFloat(data[1], result->Pitch) && ParseFloat(data[2], result->Yaw))
        {
            return result;
        }
    }

    return {};
}

template<>
Core::SharedPtr<Red::Vector3> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::Struct && aValue->data.size() == 3)
    {
        auto& data = aValue->data;
        auto result = Core::MakeShared<Red::Vector3>();

        if (ParseFloat(data[0], result->X) && ParseFloat(data[1], result->Y) && ParseFloat(data[2], result->Z))
        {
            return result;
        }
    }

    return {};
}

template<>
Core::SharedPtr<Red::Vector2> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::Struct && aValue->data.size() == 2)
    {
        auto& data = aValue->data;
        auto result = Core::MakeShared<Red::Vector2>();

        if (ParseFloat(data[0], result->X) && ParseFloat(data[1], result->Y))
        {
            return result;
        }
    }

    return {};
}

template<>
Core::SharedPtr<Red::Color> ConvertValue(const Red::TweakValuePtr& aValue)
{
    if (aValue->type == Red::ETweakValueType::Struct && aValue->data.size() == 4)
    {
        auto& data = aValue->data;
        auto result = Core::MakeShared<Red::Color>();

        if (ParseInt(data[0], result->Red) && ParseInt(data[1], result->Green)
            && ParseInt(data[2], result->Blue) && ParseInt(data[3], result->Alpha))
        {
            return result;
        }
    }

    return {};
}

template<typename T>
Core::SharedPtr<Red::DynArray<T>> ConvertValue(const Core::Vector<Red::TweakValuePtr>& aValues)
{
    auto array = Core::MakeShared<Red::DynArray<T>>();

    for (const auto& value : aValues)
    {
        const auto item = ConvertValue<T>(value);

        if (!value)
            return {};

        array->PushBack(*item);
    }

    return array;
}
}

Core::SharedPtr<void> App::RedReader::MakeValue(const App::RedReader::FlatStatePtr& aState,
                                                const Red::TweakValuePtr& aValue)
{
    const auto& type = aState->isArray ? aState->elementType : aState->resolvedType;

    switch (type->GetName())
    {
    case Red::ERTDBFlatType::Int: return ConvertValue<int>(aValue);
    case Red::ERTDBFlatType::Float: return ConvertValue<float>(aValue);
    case Red::ERTDBFlatType::Bool: return ConvertValue<bool>(aValue);
    case Red::ERTDBFlatType::String: return ConvertValue<Red::CString>(aValue);
    case Red::ERTDBFlatType::CName: return ConvertValue<Red::CName>(aValue);
    case Red::ERTDBFlatType::LocKey: return ConvertValue<Red::LocKeyWrapper>(aValue);
    case Red::ERTDBFlatType::ResRef: return ConvertValue<Red::ResourceAsyncReference<>>(aValue);
    case Red::ERTDBFlatType::TweakDBID: return ConvertValue<Red::TweakDBID>(aValue);
    case Red::ERTDBFlatType::Quaternion: return ConvertValue<Red::Quaternion>(aValue);
    case Red::ERTDBFlatType::EulerAngles: return ConvertValue<Red::EulerAngles>(aValue);
    case Red::ERTDBFlatType::Vector3: return ConvertValue<Red::Vector3>(aValue);
    case Red::ERTDBFlatType::Vector2: return ConvertValue<Red::Vector2>(aValue);
    case Red::ERTDBFlatType::Color: return ConvertValue<Red::Color>(aValue);
    }

    return {};
}

Core::SharedPtr<void> App::RedReader::MakeValue(const App::RedReader::FlatStatePtr& aState,
                                                const Core::Vector<Red::TweakValuePtr>& aValues)
{
    if (!aState->isArray)
    {
        if (aValues.size() != 1)
            return {};

        return MakeValue(aState, aValues.front());
    }

    switch (aState->resolvedType->GetName())
    {
    case Red::ERTDBFlatType::IntArray: return ConvertValue<int>(aValues);
    case Red::ERTDBFlatType::FloatArray: return ConvertValue<float>(aValues);
    case Red::ERTDBFlatType::BoolArray: return ConvertValue<bool>(aValues);
    case Red::ERTDBFlatType::StringArray: return ConvertValue<Red::CString>(aValues);
    case Red::ERTDBFlatType::CNameArray: return ConvertValue<Red::CName>(aValues);
    case Red::ERTDBFlatType::LocKeyArray: return ConvertValue<Red::LocKeyWrapper>(aValues);
    case Red::ERTDBFlatType::ResRefArray: return ConvertValue<Red::ResourceAsyncReference<>>(aValues);
    case Red::ERTDBFlatType::TweakDBIDArray: return ConvertValue<Red::TweakDBID>(aValues);
    case Red::ERTDBFlatType::QuaternionArray: return ConvertValue<Red::Quaternion>(aValues);
    case Red::ERTDBFlatType::EulerAnglesArray: return ConvertValue<Red::EulerAngles>(aValues);
    case Red::ERTDBFlatType::Vector3Array: return ConvertValue<Red::Vector3>(aValues);
    case Red::ERTDBFlatType::Vector2Array: return ConvertValue<Red::Vector2>(aValues);
    case Red::ERTDBFlatType::ColorArray: return ConvertValue<Red::Color>(aValues);
    }

    return {};
}
