#include "ScriptablePropertyManager.hpp"
#include "ScriptableTweakDBRecord.hpp"

#include <spdlog/fmt/bundled/ranges.h>

namespace App
{
std::string ScriptablePropertyGetter::GetFunctionBaseName(const std::string& aName) const
{
    return aName.substr(m_prefixLength, aName.length() - m_prefixLength - m_suffixLength);
}

std::string ScriptablePropertyGetter::GetFunctionName(const std::string& aName) const
{
    return std::string(m_prefix).append(aName).append(m_suffix);
}

Red::TweakDBID ScriptablePropertyGetter::GetFlatID(Red::Instance aInstance, const Context* aContext) const
{
    return static_cast<ScriptableTweakDBRecord*>(aInstance)->recordID + aContext->appendix;
}

template<template<typename> typename THandle>
    requires IsHandleType<THandle>
std::optional<Red::DynArray<THandle<Red::TweakDBRecord>>> ScriptablePropertyGetter::GetRecordArray(
    const Red::Value<>& aValue, const Context* aContext) const
{
    if (!aContext->typeSpec->foreignType || !Red::IsArrayType(aValue.type))
        return nullptr;

    const auto* flatArrayType = Red::ToArrayType(aValue.type);

    if (!flatArrayType)
        return nullptr;

    const auto count = flatArrayType->GetLength(aValue.instance);
    auto outArray = Red::DynArray<THandle<Red::TweakDBRecord>>(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        const auto recordID = *static_cast<Red::TweakDBID*>(flatArrayType->GetElement(aValue.instance, i));
        const auto record = aContext->tweakManager->GetRecord(recordID);

        if (record && record->GetType()->IsA(aContext->typeSpec->foreignType))
            outArray.At(i) = record;
        else
            outArray.At(i) = nullptr;
    }

    return std::move(outArray);
}

Red::CName RecordArrayGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                              const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto VoidName = Red::GetTypeNameStr<void>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(VoidName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(Red::GetWHandleArrayType(aPropSpec->typeSpec->foreignType)->GetName().ToString());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void RecordArrayGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                         const Context* aContext) const
{
    (void)aOut;

    RecordArray* out;
    Red::GetParameter(aFrame, &out);

    ++aFrame->code; // ParamEnd

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (const auto result = GetRecordArray<Red::WeakHandle>(flat, aContext))
        *out = *result;
}

void RecordArrayGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                                const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetWHandleArrayTypeName<Red::CName>(aPropSpec->typeSpec->foreignType), "outList", true);
}

Red::CName RecordArrayContainsGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                                      const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto BoolName = Red::GetTypeNameStr<bool>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(BoolName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(Red::GetWHandleType(aPropSpec->typeSpec->foreignType)->GetName().ToString());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void RecordArrayContainsGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                                 const Context* aContext) const
{
    Red::WeakHandle<Red::TweakDBRecord>* item;
    Red::GetParameter(aFrame, &item);

    ++aFrame->code; // ParamEnd

    if (!aOut)
        return;

    auto& out = *static_cast<bool*>(aOut);

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (const auto result = GetRecordArray<Red::WeakHandle>(flat, aContext))
    {
        for (const auto& record : *result)
        {
            if (record.instance->recordID == item->instance->recordID &&
                record.instance->GetType() == item->instance->GetType())
            {
                out = true;
                return;
            }
        }
    }

    out = false;
}

void RecordArrayContainsGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                                        const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetWHandleTypeName<Red::CName>(aPropSpec->typeSpec->foreignType), "item");
    aFunction->SetReturnType(Red::GetTypeName<bool>());
}

Red::CName RecordItemGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto IntName = Red::GetTypeNameStr<int>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(Red::GetWHandleTypeName<Red::CName>(aPropSpec->typeSpec->foreignType).ToString());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(IntName.data());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void RecordItemGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                        const Context* aContext) const
{
    int index;
    Red::GetParameter(aFrame, &index);

    ++aFrame->code; // ParamEnd

    if (!aOut)
        return;

    if (index < 0)
        return;

    auto& out = *static_cast<RecordWHandle*>(aOut);

    if (const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext)))
    {
        if (const auto result = GetRecordArray<Red::WeakHandle>(flat, aContext))
        {
            if (index < result->Size())
            {
                if (const auto record = result->At(index);
                    record.instance->GetType()->IsA(aContext->typeSpec->foreignType))
                {
                    out = result->At(index);
                }
            }
        }
    }
}

void RecordItemGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                               const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetTypeName<int>(), "index");
    aFunction->SetReturnType(Red::GetWHandleTypeName<Red::CName>(aPropSpec->typeSpec->foreignType));
}

Red::CName RecordItemHandleGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                                   const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto IntName = Red::GetTypeNameStr<int>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(Red::GetHandleType(aPropSpec->typeSpec->foreignType)->GetName().ToString());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(IntName.data());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void RecordItemHandleGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                              const Context* aContext) const
{
    int index;
    Red::GetParameter(aFrame, &index);

    ++aFrame->code; // ParamEnd

    if (!aOut)
        return;

    if (index < 0)
        return;

    RecordHandle& out = *static_cast<RecordHandle*>(aOut);

    if (const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext)))
    {
        if (const auto result = GetRecordArray<Red::Handle>(flat, aContext))
        {
            if (index < result->Size())
            {
                if (const auto record = result->At(index);
                    record.instance->GetType()->IsA(aContext->typeSpec->foreignType))
                {
                    out = result->At(index);
                }
            }
        }
    }
}

void RecordItemHandleGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                                     const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetTypeName<int>(), "index");
    aFunction->SetReturnType(Red::GetHandleTypeName<Red::CName>(aPropSpec->typeSpec->foreignType));
}

Red::CName RecordGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                         const ScriptablePropertySpecPtr& aPropSpec) const
{
    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(Red::GetWHandleType(aPropSpec->typeSpec->foreignType)->GetName().ToString());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void RecordGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                    const Context* aContext) const
{
    ++aFrame->code;

    if (!aOut)
        return;

    RecordWHandle& out = *static_cast<RecordWHandle*>(aOut);

    if (const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext)))
    {
        const auto* id = static_cast<Red::TweakDBID*>(flat.instance);

        if (const auto record = aContext->tweakManager->GetRecord(*id);
            record && record->GetType()->IsA(aContext->typeSpec->foreignType))
        {
            out = record;
        }
    }
}

void RecordGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                           const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->SetReturnType(Red::GetWHandleTypeName<Red::CName>(aPropSpec->typeSpec->foreignType));
}

Red::CName RecordHandleGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                               const ScriptablePropertySpecPtr& aPropSpec) const
{
    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(Red::GetHandleType(aPropSpec->typeSpec->foreignType)->GetName().ToString());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void RecordHandleGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                          const Context* aContext) const
{
    ++aFrame->code;

    if (!aOut)
        return;

    RecordHandle& out = *static_cast<RecordHandle*>(aOut);

    if (const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext)))
    {
        const auto* id = static_cast<Red::TweakDBID*>(flat.instance);

        if (const auto record = aContext->tweakManager->GetRecord(*id);
            record && record->GetType()->IsA(aContext->typeSpec->foreignType))
        {
            out = record;
        }
    }
}

void RecordHandleGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                                 const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->SetReturnType(Red::GetHandleTypeName<Red::CName>(aPropSpec->typeSpec->foreignType));
}

Red::CName ArrayCountGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto IntName = Red::GetTypeNameStr<int>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(IntName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void ArrayCountGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                        const Context* aContext) const
{
    ++aFrame->code;

    if (!aOut)
        return;

    uint32_t& out = *static_cast<uint32_t*>(aOut);

    if (const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));
        flat.type->GetType() == Red::ERTTIType::Array)
    {
        const auto* arrayType = Red::ToArrayType(flat.type);
        out = arrayType->GetLength(flat.instance);
    }
    else
    {
        out = 0;
    }
}

void ArrayCountGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                               const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->SetReturnType(Red::GetTypeName<int>());
}

Red::CName ArrayItemGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                            const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto IntName = Red::GetTypeNameStr<int>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(Red::GetInnerTypeName(aPropSpec->typeSpec->propertyType).ToString());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(IntName.data());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void ArrayItemGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                       const Context* aContext) const
{
    int index;
    Red::GetParameter(aFrame, &index);

    ++aFrame->code; // ParamEnd

    if (!aOut)
        return;

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != aContext->typeSpec->propertyType || flat.type->GetType() != Red::ERTTIType::Array)
        return;

    const auto* arrayType = Red::ToArrayType(flat.type);
    const auto* innerType = arrayType->GetInnerType();
    const auto length = arrayType->GetLength(flat.instance);

    if (index >= 0 && static_cast<uint32_t>(index) < length)
    {
        innerType->Assign(aOut, arrayType->GetElement(flat.instance, index));
    }
}

void ArrayItemGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                              const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetTypeName<int>(), "index");
    aFunction->SetReturnType(Red::GetInnerTypeName(aPropSpec->typeSpec->propertyType));
}

Red::CName ArrayContainsGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                                const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto BoolName = Red::GetTypeNameStr<bool>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(BoolName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(Red::GetInnerTypeName(aPropSpec->typeSpec->propertyType).ToString());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void ArrayContainsGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                           const Context* aContext) const
{
    void* item;
    Red::GetParameter(aFrame, &item);

    ++aFrame->code; // ParamEnd

    if (!aOut)
        return;

    bool& out = *static_cast<bool*>(aOut);

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != aContext->typeSpec->propertyType || flat.type->GetType() != Red::ERTTIType::Array)
    {
        out = false;
        return;
    }

    const auto* arrayType = Red::ToArrayType(flat.type);
    auto* innerType = arrayType->GetInnerType();
    const auto length = arrayType->GetLength(flat.instance);

    for (uint32_t i = 0; i < length; ++i)
    {
        if (innerType->IsEqual(arrayType->GetElement(flat.instance, i), item))
        {
            out = true;
            return;
        }
    }

    out = false;
}

void ArrayContainsGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                                  const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetInnerTypeName(aPropSpec->typeSpec->propertyType), "item");
    aFunction->SetReturnType(Red::GetTypeName<bool>());
}

Red::CName ResRefArrayGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                              const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto ResRefArrayName = Red::GetTypeNameStr<Red::DynArray<Red::ResRef>>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(ResRefArrayName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void ResRefArrayGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                         const Context* aContext) const
{
    static const auto RaRefArrayType =
        Red::ToArrayType(Red::TypeLocator<Red::GetTypeName<Red::DynArray<Red::RaRef<Red::CResource>>>()>::Get());

    ++aFrame->code;

    if (!aOut)
        return;

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != RaRefArrayType)
        return;

    auto* inArray = static_cast<Red::DynArray<Red::RaRef<Red::CResource>>*>(flat.instance);
    auto* outArray = static_cast<Red::DynArray<Red::ResRef>*>(aOut);

    *outArray = Red::DynArray<Red::ResRef>(inArray->Size());

    for (uint32_t i = 0; i < inArray->Size(); ++i)
        outArray->At(i) = Red::ResRef(inArray->At(i));
}

void ResRefArrayGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                                const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->SetReturnType(Red::GetTypeName<Red::DynArray<Red::ResRef>>());
}

Red::CName ResRefItemGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto IntName = Red::GetTypeNameStr<int>();
    static constexpr auto ResRefName = Red::GetTypeNameStr<Red::ResRef>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(ResRefName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(IntName.data());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void ResRefItemGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                        const Context* aContext) const
{
    static const auto RaRefArrayType =
        Red::ToArrayType(Red::TypeLocator<Red::GetTypeName<Red::DynArray<Red::RaRef<Red::CResource>>>()>::Get());

    int index;
    Red::GetParameter(aFrame, &index);

    ++aFrame->code;

    if (index < 0)
        return;

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != RaRefArrayType)
        return;

    auto& out = *static_cast<Red::ResRef*>(aOut);
    const auto* inArray = static_cast<Red::DynArray<Red::RaRef<Red::CResource>>*>(flat.instance);

    if (const auto length = inArray->Size(); index >= static_cast<int>(length))
        return;

    out = Red::ResRef(inArray->At(index));
}

void ResRefItemGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                               const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetTypeName<int>(), "index");
    aFunction->SetReturnType(Red::GetTypeName<Red::ResRef>());
}

Red::CName ResRefGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                         const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto ResRefName = Red::GetTypeNameStr<Red::ResRef>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(ResRefName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void ResRefGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                    const Context* aContext) const
{
    static const auto RaRefType = Red::TypeLocator<Red::GetTypeName<Red::RaRef<Red::CResource>>()>::Get();

    ++aFrame->code;

    if (!aOut)
        return;

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != RaRefType)
        return;

    Red::ResRef& out = *static_cast<Red::ResRef*>(aOut);
    const Red::RaRef<Red::CResource>& in = *static_cast<Red::RaRef<Red::CResource>*>(flat.instance);

    out = Red::ResRef(in);
}

void ResRefGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                           const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->SetReturnType(Red::GetTypeName<Red::ResRef>());
}

Red::CName LocKeyGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                         const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto CNameName = Red::GetTypeNameStr<Red::CName>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(CNameName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void LocKeyGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                    const Context* aContext) const
{
    static const auto LocKeyType = Red::TypeLocator<Red::GetTypeName<Red::LocKeyWrapper>()>::Get();

    ++aFrame->code;

    if (!aOut)
        return;

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != LocKeyType)
        return;

    Red::CName& out = *static_cast<Red::CName*>(aOut);
    const Red::LocKeyWrapper& in = *static_cast<Red::LocKeyWrapper*>(flat.instance);

    out = Red::CName(in.primaryKey);
}

void LocKeyGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                           const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->SetReturnType(Red::GetTypeName<Red::CName>());
}

Red::CName LocKeyArrayGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                              const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto CNameArrayName = Red::GetTypeNameStr<Red::DynArray<Red::CName>>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(CNameArrayName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void LocKeyArrayGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                         const Context* aContext) const
{
    static const auto LocKeyArrayType =
        Red::ToArrayType(Red::TypeLocator<Red::GetTypeName<Red::DynArray<Red::LocKeyWrapper>>()>::Get());

    ++aFrame->code;

    if (!aOut)
        return;

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != LocKeyArrayType)
        return;

    const auto length = LocKeyArrayType->GetLength(flat.instance);

    auto& inArray = *static_cast<Red::DynArray<Red::LocKeyWrapper>*>(flat.instance);
    auto& outArray = *static_cast<Red::DynArray<Red::CName>*>(aOut);

    outArray = Red::DynArray<Red::CName>(length);

    for (uint32_t i = 0; i < length; ++i)
        outArray.At(i) = Red::CName(inArray.At(i).primaryKey);
}

void LocKeyArrayGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                                const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->SetReturnType(Red::GetTypeName<Red::DynArray<Red::CName>>());
}

Red::CName LocKeyItemGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                             const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto IntName = Red::GetTypeNameStr<int>();
    static constexpr auto CNameName = Red::GetTypeNameStr<Red::CName>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(CNameName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(IntName.data());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void LocKeyItemGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                        const Context* aContext) const
{
    static const auto LocKeyArrayType =
        Red::ToArrayType(Red::TypeLocator<Red::GetTypeName<Red::DynArray<Red::LocKeyWrapper>>()>::Get());

    int index;
    Red::GetParameter(aFrame, &index);

    ++aFrame->code;

    if (index < 0)
        return;

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != LocKeyArrayType)
        return;

    if (const auto length = LocKeyArrayType->GetLength(flat.instance); index >= static_cast<int>(length))
        return;

    auto& out = *static_cast<Red::CName*>(aOut);
    auto& inArray = *static_cast<Red::DynArray<Red::LocKeyWrapper>*>(flat.instance);

    out = Red::CName(inArray.At(index).primaryKey);
}

void LocKeyItemGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                               const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetTypeName<int>(), "index");
    aFunction->SetReturnType(Red::GetTypeName<Red::CName>());
}

Red::CName LocKeyArrayContainsGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                                      const ScriptablePropertySpecPtr& aPropSpec) const
{
    static constexpr auto BoolName = Red::GetTypeNameStr<bool>();

    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(BoolName.data());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));
    segments.emplace_back(Red::GetTypeNameStr<Red::CName>().data());

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void LocKeyArrayContainsGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                                 const Context* aContext) const
{
    static const auto LocKeyArrayType =
        Red::ToArrayType(Red::TypeLocator<Red::GetTypeName<Red::DynArray<Red::LocKeyWrapper>>()>::Get());

    Red::CName item;
    Red::GetParameter(aFrame, &item);

    ++aFrame->code; // ParamEnd

    if (!aOut)
        return;

    auto& out = *static_cast<bool*>(aOut);

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != LocKeyArrayType)
    {
        out = false;
        return;
    }

    const auto& inArray = *static_cast<Red::DynArray<Red::LocKeyWrapper>*>(flat.instance);

    for (uint32_t i = 0; i < inArray.Size(); ++i)
    {
        if (inArray.At(i).primaryKey == item)
        {
            out = true;
            return;
        }
    }

    out = false;
}

void LocKeyArrayContainsGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                                        const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->AddParam(Red::GetTypeName<Red::CName>(), "item");
    aFunction->SetReturnType(Red::GetTypeName<bool>());
}

Red::CName ValueGetter::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                        const ScriptablePropertySpecPtr& aPropSpec) const
{
    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(aPropSpec->typeSpec->propertyType->GetName().ToString());
    segments.emplace_back(GetFunctionName(aPropSpec->functionName));

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

void ValueGetter::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                   const Context* aContext) const
{
    ++aFrame->code;

    if (!aOut)
        return;

    const auto flat = aContext->tweakManager->GetFlat(GetFlatID(aInstance, aContext));

    if (flat.type != aContext->typeSpec->flatType)
        return;

    aContext->typeSpec->propertyType->Assign(aOut, flat.instance);
}

void ValueGetter::ConfigureScriptFunction(Red::CClassFunction* aFunction,
                                          const ScriptablePropertySpecPtr& aPropSpec) const
{
    aFunction->SetReturnType(aPropSpec->typeSpec->propertyTypeName);
}

ScriptablePropertyManager::ScriptablePropertyManager(const Core::DeferredPtr<Red::TweakDBManager>& aManager)
    : m_manager(aManager)
    , m_rtti(Red::CRTTISystem::Get())
{
}

void ScriptablePropertyManager::RegisterInvocationHandler()
{
    auto* function = Red::CGlobalFunction::Create(InvocationHandlerName, InvocationHandlerName, &HandleInvocation);
    m_rtti->RegisterFunction(function);
    m_invocationHandler = function;
}

bool ScriptablePropertyManager::AdaptFunction(const ScriptableRecordSpecPtr& aRecordSpec, Red::CClassFunction* aFunc)
{
    const auto functionHash = GetFunctionHash(aRecordSpec, aFunc);

    std::shared_lock lockR(m_functionsMutex);

    if (!m_functions.contains(aRecordSpec->cname))
        return false;

    auto& it = m_functions.at(aRecordSpec->cname);

    if (!it.contains(functionHash))
        return false;

    auto& entry = it.at(functionHash);

    const auto* handler = GetPropertyGetter(entry->type);
    const auto baseFunctionName = handler->GetFunctionBaseName(aFunc->shortName.ToString());

    if (const auto propSpec = aRecordSpec->FindPropertyByFunctionName(baseFunctionName))
    {
        if (!propSpec->isCreated)
            return false;

        const auto context = GetContext(aRecordSpec, propSpec);
        ReplaceByteCode(aFunc, entry, context);
        return true;
    }

    return false;
}

void ScriptablePropertyManager::CreateFunctions(const ScriptableRecordSpecPtr& aRecordSpec)
{
    if (!aRecordSpec->type)
        return;

    for (const auto& propSpec : aRecordSpec->props | std::views::values)
        CreateFunctions(aRecordSpec, propSpec);
}

void ScriptablePropertyManager::CreateFunctions(const ScriptableRecordSpecPtr& aRecordSpec,
                                                const ScriptablePropertySpecPtr& aPropSpec)
{
    for (const auto getterType : GetGetterTypes(aPropSpec))
        CreateFunction(getterType, aRecordSpec, aPropSpec);
}

void ScriptablePropertyManager::DeleteFunctions(const ScriptableRecordSpecPtr& aRecordSpec,
                                                const ScriptablePropertySpecPtr& aPropSpec)
{
    if (!aRecordSpec->type || !aRecordSpec->isDeleted)
        return;

    if (!aPropSpec->isDeleted)
        return;

    for (const auto getterType : GetGetterTypes(aPropSpec))
        DeleteFunction(getterType, aRecordSpec, aPropSpec);
}

void ScriptablePropertyManager::HandleInvocation(Red::IScriptable* aInstance, Red::CStackFrame* aFrame, void* aOut,
                                                 int64_t)
{
    if (!aInstance || !aFrame)
        return;

    const auto context = ParseContext(aFrame);
    const auto getterType = ParseGetterType(aFrame);

    GetPropertyGetter(getterType)->HandleInvocation(aInstance, aFrame, aOut, context);
}

Context* ScriptablePropertyManager::ParseContext(Red::CStackFrame* aFrame)
{
    static constexpr auto PtrSize = sizeof(void*);

    auto* context = *reinterpret_cast<Context**>(aFrame->code);
    aFrame->code += PtrSize;
    return context;
}

GetterType ScriptablePropertyManager::ParseGetterType(Red::CStackFrame* aFrame)
{
    static constexpr auto TypeSize = sizeof(GetterType);

    const auto type = *reinterpret_cast<GetterType*>(aFrame->code);
    aFrame->code += TypeSize;
    return type;
}

ScriptablePropertyGetter* ScriptablePropertyManager::GetPropertyGetter(const GetterType aType)
{
    // clang-format off
    switch (aType)
    {
    case GetterType::GetRecordArray: return &s_recordArrayGetter;
    case GetterType::RecordArrayContains: return &s_recordArrayContainsGetter;
    case GetterType::GetRecordItem: return &s_recordItemGetter;
    case GetterType::GetRecordItemHandle: return &s_recordItemHandleGetter;
    case GetterType::GetRecord: return &s_recordGetter;
    case GetterType::GetRecordHandle: return &s_recordHandleGetter;
    case GetterType::GetArrayCount: return &s_arrayCountGetter;
    case GetterType::GetArrayItem: return &s_arrayItemGetter;
    case GetterType::ArrayContains: return &s_arrayContainsGetter;
    case GetterType::GetResRefArray: return &s_resRefArrayGetter;
    case GetterType::GetResRefItem: return &s_resRefItemGetter;
    case GetterType::GetResRef: return &s_resRefGetter;
    case GetterType::GetLocKey: return &s_locKeyGetter;
    case GetterType::GetLocKeyArray: return &s_locKeyArrayGetter;
    case GetterType::GetLocKeyItem: return &s_locKeyItemGetter;
    case GetterType::LocKeyArrayContains: return &s_locKeyArrayContainsGetter;
    case GetterType::Get: return &s_valueGetter;
    default: return nullptr;
    }
    // clang-format on
}

std::span<const GetterType> ScriptablePropertyManager::GetGetterTypes(const ScriptablePropertySpecPtr& aPropSpec)
{
    if (aPropSpec->typeSpec->isArray && aPropSpec->typeSpec->isForeignKey)
        return ForeignKeyArrayGetters;

    if (!aPropSpec->typeSpec->isArray && aPropSpec->typeSpec->isForeignKey)
        return ForeignKeyGetters;

    if (aPropSpec->typeSpec->isArray && aPropSpec->typeSpec->isResRef)
        return ResRefArrayGetters;

    if (aPropSpec->typeSpec->isArray && aPropSpec->typeSpec->isLocKey)
        return LocKeyArrayGetters;

    if (aPropSpec->typeSpec->isArray)
        return ArrayGetters;

    if (aPropSpec->typeSpec->isResRef)
        return ResRefGetters;

    if (aPropSpec->typeSpec->isLocKey)
        return LocKeyGetters;

    return ValueGetters;
}

const Context* ScriptablePropertyManager::GetContext(const ScriptableRecordSpecPtr& aRecordSpec,
                                                     const ScriptablePropertySpecPtr& aPropSpec)
{
    {
        std::shared_lock lockRW(m_contextsMutex);
        if (const auto it = m_contexts.find(aRecordSpec->cname); it != m_contexts.end())
        {
            if (const auto it2 = it->second.find(aPropSpec->cname); it2 != it->second.end())
            {
                return it2->second;
            }
        }
    }

    auto* context = Red::Memory::New<Red::Memory::DefaultAllocator, Context>();

    if (!context)
        return nullptr;

    context->tweakManager = m_manager;
    context->typeSpec = aPropSpec->typeSpec;
    context->appendix = aPropSpec->appendix;

    {
        std::unique_lock lockRW(m_contextsMutex);
        m_contexts[aRecordSpec->cname][aPropSpec->cname] = context;
    }

    return context;
}

bool ScriptablePropertyManager::CreateFunction(const GetterType aType, const ScriptableRecordSpecPtr& aRecordSpec,
                                               const ScriptablePropertySpecPtr& aPropSpec)
{
    const auto* handler = GetPropertyGetter(aType);

    if (!handler)
    {
        LogError(R"(Unsupported getter type "{}" for record "{}" property "{}")", static_cast<uint32_t>(aType),
                 aRecordSpec->aliasName, aPropSpec->name);
        return false;
    }

    const auto context = GetContext(aRecordSpec, aPropSpec);

    const auto name = handler->GetFunctionName(aPropSpec->functionName);

    LogDebug(R"(Creating function "{}::{}"...)", aRecordSpec->aliasName, name);

    auto* function = Red::CClassFunction::Create(aRecordSpec->type, name.c_str(), name.c_str(), &HandleInvocation);
    handler->ConfigureScriptFunction(function, aPropSpec);
    aRecordSpec->type->RegisterFunction(function);
    function->flags.isNative = false;
    Red::MarkSpecial(function);

    const auto entry = Core::MakeShared<FunctionEntry>();
    entry->type = aType;
    entry->function = function;

    ReplaceByteCode(function, entry, context);

    std::unique_lock lockRW(m_functionsMutex);
    m_functions[aRecordSpec->cname][handler->GetFunctionHash(aRecordSpec, aPropSpec)] = entry;

    return true;
}

bool ScriptablePropertyManager::DeleteFunction(const GetterType aType, const ScriptableRecordSpecPtr& aRecordSpec,
                                               const ScriptablePropertySpecPtr& aPropSpec)
{
    const auto* handler = GetPropertyGetter(aType);

    if (!handler)
        return false;

    if (!m_functions.contains(aRecordSpec->cname))
        return false;

    auto& recordEntry = m_functions.at(aRecordSpec->cname);

    const auto hash = handler->GetFunctionHash(aRecordSpec, aPropSpec);

    if (!recordEntry.contains(hash))
        return false;

    LogDebug(R"(Deleting function "{}::{}".)", aRecordSpec->aliasName,
             handler->GetFunctionName(aPropSpec->functionName));

    TruncateByteCode(recordEntry.at(hash));

    return true;
}

void ScriptablePropertyManager::ReplaceByteCode(Red::CClassFunction* aFunction,
                                                const Core::SharedPtr<FunctionEntry>& aEntry,
                                                const Context* aContext) const
{
    aEntry->bytecode = CreateByteCode(aEntry->type, aContext, aFunction);
    aFunction->bytecode.bytecode.buffer.data = aEntry->bytecode.data;
    aFunction->bytecode.bytecode.buffer.size = aEntry->bytecode.size;
}

void ScriptablePropertyManager::TruncateByteCode(const Core::SharedPtr<FunctionEntry>& aEntry)
{
    aEntry->bytecode = {};
    aEntry->function->bytecode.bytecode.buffer.data = aEntry->bytecode.data;
    aEntry->function->bytecode.bytecode.buffer.size = aEntry->bytecode.size;
}

Red::RawBuffer ScriptablePropertyManager::CreateByteCode(const GetterType aGetterType, const Context* aContext,
                                                         Red::CClassFunction* aFunction) const
{
    constexpr uint8_t ParamOp = 25;
    constexpr uint8_t CallStaticOp = 36;
    constexpr uint8_t ParamEndOp = 38;
    constexpr uint8_t ReturnOp = 39;
    constexpr uint32_t OpSize = sizeof(char);
    constexpr uint32_t OffsetSize = sizeof(uint16_t);
    constexpr uint32_t FlagsSize = sizeof(uint16_t);
    constexpr uint32_t PointerSize = sizeof(void*);
    constexpr uint32_t GetterTypeSize = sizeof(GetterType);
    constexpr uint32_t BaseCodeSize = OpSize + OffsetSize * 2 + PointerSize + FlagsSize + OpSize;
    constexpr uint16_t BaseExitOffset = BaseCodeSize - OpSize - OffsetSize;

    const uint32_t extraCodeSize = aFunction->params.Size() * (OpSize + PointerSize) + PointerSize + GetterTypeSize +
                                   (aFunction->returnType ? 1 : 0);
    const uint32_t finalCodeSize = BaseCodeSize + extraCodeSize;
    const uint16_t finalExitOffset = BaseExitOffset + extraCodeSize;

    Red::RawBuffer bytecode;
    bytecode.Initialize(Red::Memory::EngineAllocator::Get(), finalCodeSize);
    auto* code = static_cast<uint8_t*>(bytecode.data);

    if (aFunction->returnType)
    {
        *code = ReturnOp;
        code += OpSize;
    }

    *code = CallStaticOp;
    code += OpSize;

    *reinterpret_cast<uint16_t*>(code) = finalExitOffset;
    code += OffsetSize;

    *reinterpret_cast<uint16_t*>(code) = 0;
    code += OffsetSize;

    *reinterpret_cast<void**>(code) = m_invocationHandler;
    code += PointerSize;

    *reinterpret_cast<uint16_t*>(code) = 0;
    code += FlagsSize;

    *reinterpret_cast<const Context**>(code) = aContext;
    code += PointerSize;

    *reinterpret_cast<GetterType*>(code) = aGetterType;
    code += GetterTypeSize;

    for (const auto& param : aFunction->params)
    {
        *code = ParamOp;
        code += OpSize;

        *reinterpret_cast<void**>(code) = param;
        code += PointerSize;
    }

    *code = ParamEndOp;

    return bytecode;
}

Red::CName ScriptablePropertyManager::GetFunctionHash(const ScriptableRecordSpecPtr& aRecordSpec,
                                                      Red::CClassFunction* aFunction)
{
    std::vector<std::string> segments;
    segments.emplace_back(aRecordSpec->name);
    segments.emplace_back(aFunction->returnType ? aFunction->returnType->type->GetName().ToString() : "void");
    segments.emplace_back(aFunction->shortName.ToString());

    for (const auto& param : aFunction->params)
    {
        segments.emplace_back(param->type->GetName().ToString());
    }

    return fmt::format("{}", fmt::join(segments, ";")).c_str();
}

} // namespace App
