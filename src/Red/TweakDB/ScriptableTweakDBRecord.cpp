#include "Red/TweakDB/ScriptableTweakDBRecord.hpp"

namespace Red
{

ScriptableTweakDBRecord::ScriptableTweakDBRecord(const Red::RecordClass* aClass)
{
    this->nativeType = const_cast<RecordClass*>(aClass);
}

void ScriptableTweakDBRecord::sub_108()
{
}

Red::CClass* ScriptableTweakDBRecord::GetNativeType()
{
    return this->nativeType;
}

uint32_t ScriptableTweakDBRecord::GetTweakBaseHash() const
{
    return static_cast<Red::RecordClass*>(this->nativeType)->tweakBaseHash;
}

} // namespace Red
