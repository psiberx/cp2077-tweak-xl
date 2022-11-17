#pragma once

#include "Red/TweakDB/Alias.hpp"

namespace Red::TweakDB
{
class Reflection
{
public:
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
        int32_t defaultValue;
    };

    struct RecordInfo
    {
        Red::CName name;
        const Red::CClass* type;
        const Red::CClass* parent;
        Core::Map<Red::CName, Core::SharedPtr<PropertyInfo>> props;
        bool extraFlats;
        std::string shortName;
        uint32_t typeHash;

        [[nodiscard]] const PropertyInfo* GetPropInfo(Red::CName aPropName) const
        {
            const auto& propIt = props.find(aPropName);
            return propIt != props.end() ? propIt->second.get() : nullptr;
        }
    };

    Reflection();
    explicit Reflection(Instance* aTweakDb);

    const Red::CBaseRTTIType* GetFlatType(Red::CName aTypeName);
    const Red::CClass* GetRecordType(Red::CName aTypeName);
    const Red::CClass* GetRecordType(const char* aTypeName);

    const RecordInfo* GetRecordInfo(Red::CName aTypeName);
    const RecordInfo* GetRecordInfo(const Red::CClass* aType);

private:
    struct ExtraFlat
    {
        Red::CName typeName;
        Red::CName foreignTypeName;
        std::string appendix;
    };

    Core::SharedPtr<RecordInfo> CollectRecordInfo(const Red::CClass* aType, Red::TweakDBID aSampleId = 0);

    Red::TweakDBID GetRecordSampleId(const Red::CClass* aType);
    uint32_t GetRecordTypeHash(const Red::CClass* aType);
    std::string ResolvePropertyName(Red::TweakDBID aSampleId, Red::CName aGetterName);
    int32_t ResolveDefaultValue(const Red::CClass* aType, const std::string& aPropAppx);

    Instance* m_tweakDb;
    Red::CRTTISystem* m_rtti;
    Core::Map<Red::CName, Core::SharedPtr<RecordInfo>> m_resolved;
    static const Core::Map<Red::CName, Core::Vector<ExtraFlat>> s_extraFlats;
};
}
