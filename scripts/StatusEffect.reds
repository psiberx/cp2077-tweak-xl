@wrapMethod(StealthMappinController)
private final func UpdateStatusEffectIcon() {
    wrappedMethod();
    if this.m_statusEffectShowing {
        let iconRecord = TweakDBInterface.GetUIIconRecord(TDBID.Create("UIIcon." + this.m_mappin.GetStatusEffectIconPath()));
        if IsDefined(iconRecord) {
            inkImageRef.SetTexturePart(this.m_statusEffectIcon, iconRecord.AtlasPartName());
            inkImageRef.SetAtlasResource(this.m_statusEffectIcon, iconRecord.AtlasResourcePath());
        } else {
            inkImageRef.SetAtlasResource(this.m_statusEffectIcon, r"base/gameplay/gui/widgets/healthbar/atlas_buffinfo.inkatlas");
        }
    }
}
