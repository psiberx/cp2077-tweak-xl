#include "ScriptedManager.hpp"

namespace
{
Red::TweakDB::Manager* s_manager;
}

App::ScriptedManager::ScriptedManager(Red::TweakDB::Manager& aManager)
{
    s_manager = &aManager;
}

App::ScriptedManager::~ScriptedManager()
{
    s_manager = nullptr;
}

void App::ScriptedManager::SetFlat(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aRet, void*)
{
    RED4ext::TweakDBID flatID;
    RED4ext::Variant variant;

    RED4ext::GetParameter(aFrame, &flatID);
    RED4ext::GetParameter(aFrame, &variant);
    aFrame->code++;

    if (!s_manager)
        return;

    if (Red::TweakDB::RTDB::IsResRefToken(variant.GetType()))
    {
        const auto rtti = RED4ext::CRTTISystem::Get();
        const auto type = rtti->GetType(Red::TweakDB::RTDB::EFlatType::Resource);
        variant = RED4ext::Variant(type, variant.GetDataPtr());
    }
    else if (Red::TweakDB::RTDB::IsResRefTokenArray(variant.GetType()))
    {
        const auto rtti = RED4ext::CRTTISystem::Get();
        const auto type = rtti->GetType(Red::TweakDB::RTDB::EFlatType::ResourceArray);
        variant = RED4ext::Variant(type, variant.GetDataPtr());
    }

    // TODO: Convert LocKey

    auto success = s_manager->SetFlat(flatID, variant.GetType(), variant.GetDataPtr());

    if (aRet)
        *aRet = success;
}

void App::ScriptedManager::CreateRecord(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aRet, void*)
{
    RED4ext::TweakDBID recordID;
    RED4ext::CName typeName;

    RED4ext::GetParameter(aFrame, &recordID);
    RED4ext::GetParameter(aFrame, &typeName);
    aFrame->code++;

    if (!s_manager)
        return;

    auto recordType = s_manager->GetReflection().GetRecordType(typeName);

    if (!recordType)
        return;

    auto success = s_manager->CreateRecord(recordID, recordType);

    if (aRet)
        *aRet = success;
}

void App::ScriptedManager::CloneRecord(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aRet, void*)
{
    RED4ext::TweakDBID recordID;
    RED4ext::TweakDBID sourceID;

    RED4ext::GetParameter(aFrame, &recordID);
    RED4ext::GetParameter(aFrame, &sourceID);
    aFrame->code++;

    if (!s_manager)
        return;

    auto success = s_manager->CloneRecord(recordID, sourceID);

    if (aRet)
        *aRet = success;
}

void App::ScriptedManager::UpdateRecord(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aRet, void*)
{
    RED4ext::TweakDBID recordID;

    RED4ext::GetParameter(aFrame, &recordID);
    aFrame->code++;

    if (!s_manager)
        return;

    auto success = s_manager->UpdateRecord(recordID);

    if (aRet)
        *aRet = success;
}

void App::ScriptedManager::RegisterName(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, bool* aRet, void*)
{
    RED4ext::CName name;

    RED4ext::GetParameter(aFrame, &name);
    aFrame->code++;

    if (!s_manager || name.IsNone())
        return;

    auto id = RED4ext::TweakDBID(name.ToString());
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
