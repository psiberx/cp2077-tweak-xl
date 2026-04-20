#include "CustomGetterClosureRegistry.hpp"

namespace Red
{
CustomGetterClosureRegistry::~CustomGetterClosureRegistry()
{
    for (const auto& entry : m_entries)
    {
        if (entry->closure)
        {
            ffi_closure_free(entry->closure);
        }
    }
}

CustomGetterClosureRegistry::GetterFn CustomGetterClosureRegistry::CreateGetter(TweakDBManager* aManager,
                                                                                const PropertyInfo& aProperty)
{
    if (!aManager || !aProperty)
        return nullptr;

    std::scoped_lock lock(m_mutex);

    if (!m_cifReady)
    {
        if (ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, static_cast<unsigned>(m_argTypes.size()), &ffi_type_void,
                         m_argTypes.data()) != FFI_OK)
        {
            return nullptr;
        }

        m_cifReady = true;
    }

    auto entry = std::make_unique<Entry>();
    entry->context.manager = aManager;
    entry->context.property = aProperty;

    entry->closure = static_cast<ffi_closure*>(ffi_closure_alloc(sizeof(ffi_closure), &entry->executable));

    if (!entry->closure)
        return nullptr;

    if (ffi_prep_closure_loc(entry->closure, &m_cif, &FfiDispatch, &entry->context, entry->executable) != FFI_OK)
    {
        ffi_closure_free(entry->closure);
        return nullptr;
    }

    const auto function = reinterpret_cast<GetterFn>(entry->executable);
    m_entries.emplace_back(std::move(entry));
    return function;
}

void CustomGetterClosureRegistry::FfiDispatch(ffi_cif* aCif, void* aRet, void** aArgs, void* aUserData)
{
    (void)aCif;
    (void)aRet;

    const auto* context = static_cast<Context*>(aUserData);

    if (!context || !context->manager || !context->property)
        return;

    auto* instance = *static_cast<Red::IScriptable**>(aArgs[0]);
    auto* stackFrame = *static_cast<Red::CStackFrame**>(aArgs[1]);
    auto* out = *static_cast<void**>(aArgs[2]);

    if (!instance || !stackFrame || !out)
        return;

    stackFrame->code++;

    const auto* record = reinterpret_cast<Red::CustomTweakDBRecord*>(instance);

    if (const auto flat = context->manager->GetFlat(record->recordID + context->property->appendix);
        flat && flat.instance)
    {
        context->property->type->Assign(out, flat.instance);
    }
}
} // namespace Red
