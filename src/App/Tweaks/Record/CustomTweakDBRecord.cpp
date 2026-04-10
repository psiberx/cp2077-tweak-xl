#include "CustomTweakDBRecord.hpp"

namespace
{
constexpr Red::ClassLocator<App::CustomTweakDBRecord> s_CustomTweakDBRecordType;
} // namespace

namespace App
{

using namespace Red;

CustomTweakDBRecord::CustomTweakDBRecord(const TweakDBRecordInfo& aRecordInfo, TweakDBID aTweakDBID)
    : m_tweakBaseHash(aRecordInfo.typeHash)
{
    this->recordID = aTweakDBID;
}

uint32_t CustomTweakDBRecord::GetTweakBaseHash() const
{
    return m_tweakBaseHash;
}

} // namespace App
