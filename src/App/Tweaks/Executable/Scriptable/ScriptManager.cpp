#include "App/Tweaks/Executable/Scriptable/ScriptUtils.hpp"
#include "ScriptManager.hpp"

void App::ScriptManager::SetManager(Core::SharedPtr<Red::TweakDBManager> aManager)
{
    s_manager = std::move(aManager);
    s_reflection = s_manager->GetReflection();
}

void App::ScriptManager::SetFlat(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
{
    Red::TweakDBID flatID;
    Red::Variant variant;

    Red::GetParameter(aFrame, &flatID);
    Red::GetParameter(aFrame, &variant);
    aFrame->code++;

    if (!s_manager || variant.IsEmpty())
        return;

    ConvertScriptValueForFlatValue(variant, s_reflection);

    auto success = s_manager->SetFlat(flatID, variant.GetType(), variant.GetDataPtr());

    if (aRet)
    {
        *aRet = success;
    }
}

void App::ScriptManager::CreateRecord(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
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

void App::ScriptManager::CloneRecord(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
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

void App::ScriptManager::UpdateRecord(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
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

void App::ScriptManager::RegisterName(Red::IScriptable*, Red::CStackFrame* aFrame, bool* aRet, void*)
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

void App::ScriptManager::RegisterEnum(Red::TweakDBID aRecordID)
{
    if (s_manager)
    {
        s_manager->RegisterEnum(aRecordID);
    }
}

Red::Handle<App::ScriptBatch> App::ScriptManager::StartBatch()
{
    return Red::MakeHandle<ScriptBatch>(s_manager);
}
