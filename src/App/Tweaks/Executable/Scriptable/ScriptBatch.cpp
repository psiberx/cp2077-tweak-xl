#include "ScriptBatch.hpp"
#include "App/Tweaks/Executable/Scriptable/ScriptUtils.hpp"

App::ScriptBatch::ScriptBatch(Core::DeferredPtr<Red::TweakDBManager> aManager,
                              Core::DeferredPtr<Red::TweakDBReflection> aReflection)
    : m_manager(std::move(aManager))
    , m_reflection(std::move(aReflection))
{
    if (m_manager)
    {
        m_batch = m_manager->StartBatch();
    }
}

bool App::ScriptBatch::SetFlat(Red::TweakDBID aFlatID, Red::Variant& aVariant) const
{
    if (m_batch && !aVariant.IsEmpty())
    {
        ConvertScriptValueForFlatValue(aVariant);
        return m_manager->SetFlat(m_batch, aFlatID, aVariant.GetType(), aVariant.GetDataPtr());
    }

    return false;
}

bool App::ScriptBatch::CreateRecord(Red::TweakDBID aRecordID, Red::CName aTypeName) const
{
    if (m_batch && aTypeName)
    {
        return m_manager->CreateRecord(m_batch, aRecordID, Red::GetRecordType(aTypeName));
    }

    return false;
}

bool App::ScriptBatch::CloneRecord(Red::TweakDBID aRecordID, Red::TweakDBID aSourceID) const
{
    if (m_batch)
    {
        return m_manager->CloneRecord(m_batch, aRecordID, aSourceID);
    }

    return false;
}

bool App::ScriptBatch::UpdateRecord(Red::TweakDBID aRecordID) const
{
    if (m_batch)
    {
        return m_manager->UpdateRecord(m_batch, aRecordID);
    }

    return false;
}

bool App::ScriptBatch::RegisterEnum(Red::TweakDBID aRecordID) const
{
    if (m_batch && aRecordID)
    {
        m_manager->RegisterEnum(m_batch, aRecordID);
        return true;
    }

    return false;
}

bool App::ScriptBatch::RegisterName(Red::CName aName) const
{
    if (m_batch && aName)
    {
        m_manager->RegisterName(m_batch, aName.ToString(), aName.ToString());
        return true;
    }

    return false;
}

void App::ScriptBatch::Commit() const
{
    if (m_batch)
    {
        m_manager->CommitBatch(m_batch);
    }
}
