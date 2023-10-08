#include "ScriptInterface.hpp"

void App::ScriptInterface::SetReflection(Core::SharedPtr<Red::TweakDBReflection> aReflection)
{
    s_reflection = std::move(aReflection);
}

void App::ScriptInterface::GetFlat(Red::IScriptable*, Red::CStackFrame* aFrame,
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

void App::ScriptInterface::GetRecord(Red::IScriptable*, Red::CStackFrame* aFrame,
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

void App::ScriptInterface::GetRecords(Red::IScriptable*, Red::CStackFrame* aFrame,
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

void App::ScriptInterface::GetRecordCount(Red::IScriptable*, Red::CStackFrame* aFrame, uint32_t* aRet, void*)
{
    Red::CName recordTypeName;

    Red::GetParameter(aFrame, &recordTypeName);
    aFrame->code++;

    if (!aRet)
        return;

    auto records = FetchRecords(recordTypeName);

    *aRet = records->size;
}

void App::ScriptInterface::GetRecordByIndex(Red::IScriptable*, Red::CStackFrame* aFrame,
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

App::ScriptInterface::ScriptableArray* App::ScriptInterface::FetchRecords(Red::CName aTypeName)
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
