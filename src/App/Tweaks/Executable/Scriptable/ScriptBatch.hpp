#pragma once

#include "Red/TweakDB/Manager.hpp"

namespace App
{
struct ScriptBatch : Red::IScriptable
{
    ScriptBatch() = default;
    ScriptBatch(Core::SharedPtr<Red::TweakDBManager> aManager);

    bool SetFlat(Red::TweakDBID aFlatID, Red::Variant& aVariant) const;
    bool CreateRecord(Red::TweakDBID aRecordID, Red::CName aTypeName) const;
    bool CloneRecord(Red::TweakDBID aRecordID, Red::TweakDBID aSourceID) const;
    bool UpdateRecord(Red::TweakDBID aRecordID) const;
    bool RegisterEnum(Red::TweakDBID aRecordID) const;
    bool RegisterName(Red::CName aName) const;
    void Commit() const;

    Core::SharedPtr<Red::TweakDBManager> m_manager;
    Core::SharedPtr<Red::TweakDBManager::Batch> m_batch;
    Core::SharedPtr<Red::TweakDBReflection> m_reflection;

    RTTI_IMPL_TYPEINFO(App::ScriptBatch);
    RTTI_IMPL_ALLOCATOR();
};
}

RTTI_DEFINE_CLASS(App::ScriptBatch, "TweakDBBatch", {
    RTTI_METHOD(SetFlat);
    RTTI_METHOD(CreateRecord);
    RTTI_METHOD(CloneRecord);
    RTTI_METHOD(UpdateRecord);
    RTTI_METHOD(RegisterEnum);
    RTTI_METHOD(RegisterName);
    RTTI_METHOD(Commit);
});
