#include "ScriptedManager.hpp"
#include "App/Utils/Str.hpp"
#include "Red/Localization.hpp"

void App::ScriptedManager::SetManager(Core::SharedPtr<Red::TweakDBManager> aManager)
{
    s_manager = std::move(aManager);
    s_reflection = s_manager->GetReflection();
}

void App::ScriptedManager::SetFlat(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
{
    Red::TweakDBID flatID;
    Red::Variant variant;

    Red::GetParameter(aFrame, &flatID);
    Red::GetParameter(aFrame, &variant);
    aFrame->code++;

    if (!s_manager || variant.IsEmpty())
        return;

    if (s_reflection->IsResRefToken(variant.GetType()))
    {
        const auto rtti = Red::CRTTISystem::Get();
        const auto type = rtti->GetType(Red::ERTDBFlatType::ResRef);
        variant = Red::Variant(type, variant.GetDataPtr());
    }
    else if (s_reflection->IsResRefTokenArray(variant.GetType()))
    {
        const auto rtti = Red::CRTTISystem::Get();
        const auto type = rtti->GetType(Red::ERTDBFlatType::ResRefArray);
        variant = Red::Variant(type, variant.GetDataPtr());
    }
    else if (variant.GetType()->GetName() == Red::ERTDBFlatType::String)
    {
        constexpr auto Prefix = Red::LocKeyPrefix;
        constexpr auto PrefixSize = Red::LocKeyPrefixLength;

        const auto str = reinterpret_cast<Red::CString*>(variant.GetDataPtr());

        if (strncmp(str->c_str(), Prefix, PrefixSize) == 0)
        {
            const auto& value = str->c_str() + PrefixSize;
            const auto& length = str->Length() - PrefixSize;
            Red::LocKeyWrapper wrapper;

            if (!App::ParseInt(value, length, wrapper.primaryKey))
            {
                wrapper.primaryKey = Red::FNV1a64(value);
            }

            variant.Fill(s_reflection->GetFlatType(Red::ERTDBFlatType::LocKey), &wrapper);
        }
    }

    auto success = s_manager->SetFlat(flatID, variant.GetType(), variant.GetDataPtr());

    if (aRet)
    {
        *aRet = success;
    }
}

void App::ScriptedManager::CreateRecord(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
{
    Red::TweakDBID recordID;
    Red::CName typeName;

    Red::GetParameter(aFrame, &recordID);
    Red::GetParameter(aFrame, &typeName);
    aFrame->code++;

    if (!s_manager)
        return;

    auto recordType = s_reflection->GetRecordType(typeName);

    if (!recordType)
        return;

    auto success = s_manager->CreateRecord(recordID, recordType);

    if (aRet)
    {
        *aRet = success;
    }
}

void App::ScriptedManager::CloneRecord(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
{
    Red::TweakDBID recordID;
    Red::TweakDBID sourceID;

    Red::GetParameter(aFrame, &recordID);
    Red::GetParameter(aFrame, &sourceID);
    aFrame->code++;

    if (!s_manager)
        return;

    auto success = s_manager->CloneRecord(recordID, sourceID);

    if (aRet)
    {
        *aRet = success;
    }
}

void App::ScriptedManager::UpdateRecord(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
{
    Red::TweakDBID recordID;

    Red::GetParameter(aFrame, &recordID);
    aFrame->code++;

    if (!s_manager)
        return;

    auto success = s_manager->UpdateRecord(recordID);

    if (aRet)
    {
        *aRet = success;
    }
}

void App::ScriptedManager::RegisterName(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
{
    Red::CName name;

    Red::GetParameter(aFrame, &name);
    aFrame->code++;

    if (!s_manager || name.IsNone())
        return;

    auto id = Red::TweakDBID(name.ToString());
    auto str = std::string(name.ToString());

    s_manager->RegisterName(id, str);

    if (aRet)
    {
        *aRet = true;
    }
}
