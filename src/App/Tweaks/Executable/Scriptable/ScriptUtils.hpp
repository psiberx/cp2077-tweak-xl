#pragma once

#include "App/Utils/Str.hpp"
#include "Red/Localization.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
inline void ConvertScriptValueForFlatValue(Red::Variant& aVariant,
                                           const Core::SharedPtr<Red::TweakDBReflection>& aReflection)
{
    const auto& variantType = aVariant.GetType();

    if (aReflection->IsResRefToken(variantType))
    {
        const auto rtti = Red::CRTTISystem::Get();
        const auto type = rtti->GetType(Red::ERTDBFlatType::ResRef);

        aVariant = Red::Variant(type, aVariant.GetDataPtr());
    }
    else if (aReflection->IsResRefTokenArray(variantType))
    {
        const auto rtti = Red::CRTTISystem::Get();
        const auto type = rtti->GetType(Red::ERTDBFlatType::ResRefArray);

        aVariant = Red::Variant(type, aVariant.GetDataPtr());
    }
    else if (variantType->GetName() == Red::ERTDBFlatType::CName)
    {
        const auto str = reinterpret_cast<Red::CName*>(aVariant.GetDataPtr())->ToString();

        if (strncmp(str, Red::LocKeyPrefix, Red::LocKeyPrefixLength) == 0)
        {
            const auto& value = str + Red::LocKeyPrefixLength;
            const auto& length = strlen(str) - Red::LocKeyPrefixLength;
            Red::LocKeyWrapper wrapper;

            if (!App::ParseInt(value, length, wrapper.primaryKey))
            {
                wrapper.primaryKey = Red::FNV1a64(value);
            }

            aVariant.Fill(aReflection->GetFlatType(Red::ERTDBFlatType::LocKey), &wrapper);
        }
    }
    else if (variantType->GetName() == Red::ERTDBFlatType::String)
    {
        auto str = reinterpret_cast<Red::CString*>(aVariant.GetDataPtr());

        if (strncmp(str->c_str(), Red::LocKeyPrefix, Red::LocKeyPrefixLength) == 0)
        {
            const std::string_view value{str->c_str() + Red::LocKeyPrefixLength,
                                         str->Length() - Red::LocKeyPrefixLength};
            if (!IsNumeric(value))
            {
                std::string key = Red::LocKeyPrefix + std::to_string(Red::FNV1a64(value.data()));
                *str = key.data();
            }
        }
    }
}
}
