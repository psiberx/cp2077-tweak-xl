#include "App/Tweaks/Record/ScriptableTweakDBRecord.hpp"

#include "ScriptableRecordClass.hpp"

namespace App
{

void ScriptableTweakDBRecord::sub_108()
{
}

Red::CClass* ScriptableTweakDBRecord::GetNativeType()
{
    return this->nativeType;
}

uint32_t ScriptableTweakDBRecord::GetTweakBaseHash() const
{
    return reinterpret_cast<ScriptableRecordClass*>(this->nativeType)->tweakBaseHash;
}

} // namespace App
