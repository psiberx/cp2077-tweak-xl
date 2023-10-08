public native class TweakDBBatch {
    public native func SetFlat(id: TweakDBID, value: Variant) -> Bool
    public native func CreateRecord(id: TweakDBID, type: CName) -> Bool
    public native func CloneRecord(id: TweakDBID, base: TweakDBID) -> Bool
    public native func UpdateRecord(id: TweakDBID) -> Bool
    public native func RegisterName(name: CName) -> Bool
    public native func Commit()

    public func SetFlat(name: CName, value: Variant) -> Bool {
        if this.SetFlat(TDBID.Create(NameToString(name)), value) {
            this.RegisterName(name);
            return true;
        }
        return false;
    }

    public func CreateRecord(name: CName, type: CName) -> Bool {
        if this.CreateRecord(TDBID.Create(NameToString(name)), type) {
            this.RegisterName(name);
            return true;
        }
        return false;
    }

    public func CloneRecord(name: CName, base: TweakDBID) -> Bool {
        if this.CloneRecord(TDBID.Create(NameToString(name)), base) {
            this.RegisterName(name);
            return true;
        }
        return false;
    }
}
