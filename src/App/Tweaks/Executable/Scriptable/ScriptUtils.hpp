#pragma once

#include "App/Utils/Str.hpp"
#include "Red/Localization.hpp"
#include "Red/TweakDB/Manager.hpp"

namespace App
{
inline void ConvertScriptValueForFlatValue(Red::Variant& aVariant,
                                           const Core::SharedPtr<Red::TweakDBReflection>& aReflection)
{
    if (aReflection->IsResRefToken(aVariant.GetType()))
    {
        const auto rtti = Red::CRTTISystem::Get();
        const auto type = rtti->GetType(Red::ERTDBFlatType::ResRef);

        aVariant = Red::Variant(type, aVariant.GetDataPtr());
    }
    else if (aReflection->IsResRefTokenArray(aVariant.GetType()))
    {
        const auto rtti = Red::CRTTISystem::Get();
        const auto type = rtti->GetType(Red::ERTDBFlatType::ResRefArray);

        aVariant = Red::Variant(type, aVariant.GetDataPtr());
    }
    else if (aVariant.GetType()->GetName() == Red::ERTDBFlatType::String)
    {
        const auto str = reinterpret_cast<Red::CString*>(aVariant.GetDataPtr());

        if (strncmp(str->c_str(), Red::LocKeyPrefix, Red::LocKeyPrefixLength) == 0)
        {
            const auto& value = str->c_str() + Red::LocKeyPrefixLength;
            const auto& length = str->Length() - Red::LocKeyPrefixLength;
            Red::LocKeyWrapper wrapper;

            if (!App::ParseInt(value, length, wrapper.primaryKey))
            {
                wrapper.primaryKey = Red::FNV1a64(value);
            }

            aVariant.Fill(aReflection->GetFlatType(Red::ERTDBFlatType::LocKey), &wrapper);
        }
    }
}
}
