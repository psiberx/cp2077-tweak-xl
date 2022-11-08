#pragma once

#include "Alias.hpp"

#include <RED4ext/CName.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/TweakDB.hpp>

namespace Red::TweakDB
{
struct PropertyInfo
{
    Red::CName name;
    const Red::CBaseRTTIType* type;
    bool isArray;
    const Red::CBaseRTTIType* elementType;
    bool isForeignKey;
    const Red::CClass* foreignType;
    std::string appendix; // The name used to build TweakDBID of the property
    uintptr_t offset;
};

struct RecordTypeInfo
{
    Red::CName name;
    const Red::CClass* type;
    const Red::CClass* parent;
    Core::SortedMap<Red::CName, Core::SharedPtr<PropertyInfo>> props;
    std::string shortName;
    uint32_t typeHash;

    [[nodiscard]] const PropertyInfo* GetPropInfo(Red::CName aPropName) const
    {
        const auto& propIt = props.find(aPropName);
        return propIt != props.end() ? propIt->second.get() : nullptr;
    }
};

class Reflection
{
public:
    Reflection();
    explicit Reflection(Instance* aTweakDb);

    const Red::CBaseRTTIType* GetFlatType(Red::CName aTypeName);
    const Red::CClass* GetRecordType(Red::CName aTypeName);
    const Red::CClass* GetRecordType(const char* aTypeName);

    const RecordTypeInfo* GetRecordInfo(Red::CName aTypeName);
    const RecordTypeInfo* GetRecordInfo(const Red::CClass* aType);

private:
    Core::SharedPtr<RecordTypeInfo> CollectRecordInfo(const Red::CClass* aType, Red::TweakDBID aSampleId = 0);

    Red::TweakDBID GetRecordSampleId(const Red::CClass* aType);
    uint32_t GetRecordTypeHash(const Red::CClass* aType);
    std::string ResolvePropertyName(Red::TweakDBID aSampleId, Red::CName aGetterName);

    Instance* m_tweakDb;
    Red::CRTTISystem* m_rtti;
    Core::Map<Red::CName, Core::SharedPtr<RecordTypeInfo>> m_data;
};
}
