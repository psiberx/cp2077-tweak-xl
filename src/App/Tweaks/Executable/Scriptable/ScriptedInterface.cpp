#include "ScriptedInterface.hpp"

void App::ScriptedInterface::SetReflection(Core::SharedPtr<Red::TweakDBReflection> aReflection)
{
    s_reflection = std::move(aReflection);
}

void App::ScriptedInterface::GetFlat(Red::IScriptable*, Red::CStackFrame* aFrame,
                                     Red::Variant* aRet, Red::CBaseRTTIType*)
{
    Red::TweakDBID flatID;

    Red::GetParameter(aFrame, &flatID);
    aFrame->code++;

    if (!aRet)
        return;

    auto* tdb = Red::TweakDB::Get();
    if (!tdb)
        return;

    auto* flat = tdb->GetFlatValue(flatID);
    if (!flat)
        return;

    auto data = flat->GetValue();

     if (!data.value)
        return;

    aRet->Fill(data.type, data.value);
}

void App::ScriptedInterface::GetRecord(Red::IScriptable*, Red::CStackFrame* aFrame,
                                       RecordHandle* aRet, Red::CRTTIHandleType* aRetType)
{
    Red::TweakDBID recordID;

    Red::GetParameter(aFrame, &recordID);
    aFrame->code++;

    if (!aRet)
        return;

    auto* tdb = Red::TweakDB::Get();
    if (!tdb)
        return;

    ScriptableHandle record;
    tdb->TryGetRecord(recordID, record);

    if (!record)
        return;

    aRetType->Assign(aRet, &record);
}

void App::ScriptedInterface::GetRecords(Red::IScriptable*, Red::CStackFrame* aFrame,
                                        RecordArray* aRet, Red::CRTTIBaseArrayType* aRetType)
{
    Red::CName recordTypeName;

    Red::GetParameter(aFrame, &recordTypeName);
    aFrame->code++;

    if (!aRet)
        return;

    auto records = FetchRecords(s_reflection->GetRecordFullName(recordTypeName));

    if (records->size <= 0)
        return;

    aRetType->Assign(aRet, records);
}

void App::ScriptedInterface::GetRecordCount(Red::IScriptable*, Red::CStackFrame* aFrame, uint32_t* aRet, void*)
{
    Red::CName recordTypeName;

    Red::GetParameter(aFrame, &recordTypeName);
    aFrame->code++;

    if (!aRet)
        return;

    auto records = FetchRecords(recordTypeName);

    *aRet = records->size;
}

void App::ScriptedInterface::GetRecordByIndex(Red::IScriptable*, Red::CStackFrame* aFrame,
                                              RecordHandle* aRet, Red::CRTTIHandleType* aRetType)
{
    Red::CName recordTypeName;
    uint32_t recordIndex;

    Red::GetParameter(aFrame, &recordTypeName);
    Red::GetParameter(aFrame, &recordIndex);
    aFrame->code++;

    if (!aRet)
        return;

    if (recordIndex < 0)
        return;

    auto records = FetchRecords(recordTypeName);

    if (records->size <= 0 || recordIndex >= records->size)
        return;

    aRetType->Assign(aRet, &records->entries[recordIndex]);
}

App::ScriptedInterface::ScriptableArray* App::ScriptedInterface::FetchRecords(Red::CName aTypeName)
{
    auto* rtti = Red::CRTTISystem::Get();
    if (!rtti)
        return nullptr;

    auto* recordType = rtti->GetType(aTypeName);
    if (!recordType)
        return nullptr;

    auto* tdb = Red::TweakDB::Get();
    if (!tdb)
        return nullptr;

    std::shared_lock<Red::SharedMutex> _(tdb->mutex01);

    return tdb->recordsByType.Get(recordType);
}

void App::ScriptedInterface::OnExpand(Descriptor* aType)
{
    {
        auto func = aType->AddFunction(&GetRecords, "GetRecords", { .isFinal = true });
        func->AddParam("CName", "type");
        func->SetReturnType("array:handle:gamedataTweakDBRecord");
    }
    {
        auto func = aType->AddFunction(&GetRecordCount, "GetRecordCount", { .isFinal = true });
        func->AddParam("CName", "type");
        func->SetReturnType("Int32");
    }
    {
        auto func = aType->AddFunction(&GetRecordByIndex, "GetRecordByIndex", { .isFinal = true });
        func->AddParam("CName", "type");
        func->AddParam("Int32", "index");
        func->SetReturnType("handle:gamedataTweakDBRecord");
    }
    {
        auto func = aType->AddFunction(&GetRecord, "GetRecord", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->SetReturnType("handle:gamedataTweakDBRecord");
    }
    {
        auto func = aType->AddFunction(&GetFlat, "GetFlat", { .isFinal = true });
        func->AddParam("TweakDBID", "path");
        func->SetReturnType("Variant");
    }
}
