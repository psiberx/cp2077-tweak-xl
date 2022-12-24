
@addMethod(TweakDBInterface)
public final static native func GetFlat(path: TweakDBID) -> Variant

@addMethod(TweakDBInterface)
public final static native func GetRecord(path: TweakDBID) -> ref<TweakDBRecord>

@addMethod(TweakDBInterface)
public final static native func GetRecords(type: CName) -> array<ref<TweakDBRecord>>

@addMethod(TweakDBInterface)
public final static native func GetRecordCount(type: CName) -> Uint32

@addMethod(TweakDBInterface)
public final static native func GetRecordByIndex(type: CName, index: Uint32) -> ref<TweakDBRecord>

@addMethod(TweakDBInterface)
public final static func GetRecords(keys: array<TweakDBID>) -> array<ref<TweakDBRecord>> {
    let records: array<ref<TweakDBRecord>>;
    for key in keys {
        let record = TweakDBInterface.GetRecord(key);
        if IsDefined(record) {
            ArrayPush(records, record);
        }
    }
    return records;
}

@addMethod(TweakDBInterface)
public final static func GetRecordIDs(type: CName) -> array<TweakDBID> {
    let ids: array<TweakDBID>;
    for record in TweakDBInterface.GetRecords(type) {
        ArrayPush(ids, record.GetID());
    }
    return ids;
}
