#include "Red/TweakDB/CustomTweakDBRecord.hpp"

namespace Red
{

CustomTweakDBRecord::CustomTweakDBRecord(const TweakDBRecordInfo& aRecordInfo, TweakDBID aTweakDBID)
    : m_tweakBaseHash(aRecordInfo.typeHash)
    , m_type(const_cast<CClass*>(aRecordInfo.type))
{
    this->recordID = aTweakDBID;
}

void CustomTweakDBRecord::sub_108()
{
}

CClass* CustomTweakDBRecord::GetType()
{
    return m_type;
}

uint32_t CustomTweakDBRecord::GetTweakBaseHash() const
{
    return m_tweakBaseHash;
}

} // namespace Red
