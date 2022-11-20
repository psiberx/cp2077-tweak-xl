#include "ScriptedManager.hpp"

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

    if (!s_manager)
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

    // TODO: Convert LocKey

    auto success = s_manager->SetFlat(flatID, variant.GetType(), variant.GetDataPtr());

    if (aRet)
        *aRet = success;
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
        *aRet = success;
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
        *aRet = success;
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
        *aRet = success;
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
        *aRet = true;
}

void App::ScriptedManager::OnRegister(Descriptor* aType)
{
    aType->SetName("TweakDBManager");
    aType->SetFlags({ .isAbstract = true });
}

void App::ScriptedManager::OnDescribe(Descriptor* aType)
{
    {
        auto func = aType->AddFunction(&SetFlat, "SetFlat", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->AddParam("Variant", "value");
        func->SetReturnType("Bool");
    }
    {
        auto func = aType->AddFunction(&CreateRecord, "CreateRecord", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->AddParam("CName", "type");
        func->SetReturnType("Bool");
    }
    {
        auto func = aType->AddFunction(&CloneRecord, "CloneRecord", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->AddParam("TweakDBID", "base");
        func->SetReturnType("Bool");
    }
    {
        auto func = aType->AddFunction(&UpdateRecord, "UpdateRecord", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->SetReturnType("Bool");
    }
    {
        auto func = aType->AddFunction(&RegisterName, "RegisterName", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->SetReturnType("Bool");
    }
}
