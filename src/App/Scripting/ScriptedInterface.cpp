#include "ScriptedInterface.hpp"
#include "TweakDB/Types.hpp"

void App::ScriptedInterface::GetFlat(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame,
                                    RED4ext::Variant* aRet, RED4ext::CBaseRTTIType* aRetType)
{
    RED4ext::TweakDBID flatID;

    RED4ext::GetParameter(aFrame, &flatID);
    aFrame->code++;

    if (!aRet)
        return;

    auto* tdb = RED4ext::TweakDB::Get();
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

void App::ScriptedInterface::GetRecord(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame,
                                       RecordHandle* aRet, RED4ext::CRTTIHandleType* aRetType)
{
    RED4ext::TweakDBID recordID;

    RED4ext::GetParameter(aFrame, &recordID);
    aFrame->code++;

    if (!aRet)
        return;

    auto* tdb = RED4ext::TweakDB::Get();
    if (!tdb)
        return;

    ScriptableHandle record;
    tdb->TryGetRecord(recordID, record);

    if (!record)
        return;

    aRetType->Assign(aRet, &record);
}

void App::ScriptedInterface::GetRecords(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame,
                                        RecordArray* aRet, RED4ext::CRTTIBaseArrayType* aRetType)
{
    RED4ext::CName recordTypeName;

    RED4ext::GetParameter(aFrame, &recordTypeName);
    aFrame->code++;

    if (!aRet)
        return;

    auto records = FetchRecords(TweakDB::RTDB::GetRecordFullName(recordTypeName));

    if (records.size <= 0)
        return;

    aRetType->Assign(aRet, &records);
}

void App::ScriptedInterface::GetRecordCount(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame, uint32_t* aRet, void*)
{
    RED4ext::CName recordTypeName;

    RED4ext::GetParameter(aFrame, &recordTypeName);
    aFrame->code++;

    if (!aRet)
        return;

    auto records = FetchRecords(recordTypeName);

    *aRet = records.size;
}

void App::ScriptedInterface::GetRecordByIndex(RED4ext::IScriptable*, RED4ext::CStackFrame* aFrame,
                                              RecordHandle* aRet, RED4ext::CRTTIHandleType* aRetType)
{
    RED4ext::CName recordTypeName;
    uint32_t recordIndex;

    RED4ext::GetParameter(aFrame, &recordTypeName);
    RED4ext::GetParameter(aFrame, &recordIndex);
    aFrame->code++;

    if (!aRet)
        return;

    if (recordIndex < 0)
        return;

    auto records = FetchRecords(recordTypeName);

    if (records.size <= 0 || recordIndex >= records.size)
        return;

    aRetType->Assign(aRet, &records.entries[recordIndex]);
}

App::ScriptedInterface::ScriptableArray App::ScriptedInterface::FetchRecords(RED4ext::CName aTypeName)
{
    ScriptableArray records;

    auto* rtti = RED4ext::CRTTISystem::Get();
    if (!rtti)
        return records;

    auto* recordType = rtti->GetType(aTypeName);
    if (!recordType)
        return records;

    auto* tdb = RED4ext::TweakDB::Get();
    if (!tdb)
        return records;

    tdb->TryGetRecordsByType(recordType, records);

    return records;
}

void App::ScriptedInterface::OnExpand(Descriptor* aType, RED4ext::CRTTISystem* aRtti)
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
