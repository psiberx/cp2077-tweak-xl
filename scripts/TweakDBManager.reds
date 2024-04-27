public abstract native class TweakDBManager {
    public final static native func SetFlat(id: TweakDBID, value: Variant) -> Bool
    public final static native func CreateRecord(id: TweakDBID, type: CName) -> Bool
    public final static native func CloneRecord(id: TweakDBID, base: TweakDBID) -> Bool
    public final static native func UpdateRecord(id: TweakDBID) -> Bool
    public final static native func RegisterName(name: CName) -> Bool
    public final static native func StartBatch() -> ref<TweakDBBatch>

    public final static func SetFlat(name: CName, value: Variant) -> Bool {
        if TweakDBManager.SetFlat(TDBID.Create(NameToString(name)), value) {
            TweakDBManager.RegisterName(name);
            return true;
        }
        return false;
    }

    public final static func CreateRecord(name: CName, type: CName) -> Bool {
        if TweakDBManager.CreateRecord(TDBID.Create(NameToString(name)), type) {
            TweakDBManager.RegisterName(name);
            return true;
        }
        return false;
    }

    public final static func CloneRecord(name: CName, base: TweakDBID) -> Bool {
        if TweakDBManager.CloneRecord(TDBID.Create(NameToString(name)), base) {
            TweakDBManager.RegisterName(name);
            return true;
        }
        return false;
    }
}
