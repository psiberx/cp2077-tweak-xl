#include "ScriptableRecordClosureRegistry.hpp"

#include "Reflection.hpp"

namespace Red
{
ScriptableRecordClosureRegistry::~ScriptableRecordClosureRegistry()
{
    for (const auto& entry : m_entries)
    {
        if (entry->closure)
        {
            ffi_closure_free(entry->closure);
        }
    }
}

ScriptableRecordClosureRegistry::GetterFn ScriptableRecordClosureRegistry::CreateClosure(TweakDBReflection* aReflection,
                                                                                         const PropertyInfo& aProperty)
{
    if (!aReflection || !aProperty)
    {
        return nullptr;
    }

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

    auto entry = Core::MakeUnique<Entry>();
    entry->context.reflection = aReflection;
    entry->context.property = aProperty;

    entry->closure = static_cast<ffi_closure*>(ffi_closure_alloc(sizeof(ffi_closure), &entry->executable));

    if (!entry->closure)
    {
        return nullptr;
    }

    if (ffi_prep_closure_loc(entry->closure, &m_cif, &FfiDispatch, &entry->context, entry->executable) != FFI_OK)
    {
        ffi_closure_free(entry->closure);
        return nullptr;
    }

    const auto function = reinterpret_cast<GetterFn>(entry->executable);
    m_entries.emplace_back(std::move(entry));
    return function;
}

bool ScriptableRecordClosureRegistry::DestroyClosure(const GetterFn aClosure)
{
    if (!aClosure)
    {
        return false;
    }

    std::scoped_lock lock(m_mutex);

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        const auto& entry = *it;

        if (!entry || !entry->closure)
        {
            continue;
        }

        if (reinterpret_cast<GetterFn>(entry->executable) != aClosure)
        {
            continue;
        }

        ffi_closure_free(entry->closure);
        entry->closure = nullptr;
        entry->executable = nullptr;
        m_entries.erase(it);

        return true;
    }

    return false;
}

void ScriptableRecordClosureRegistry::FfiDispatch(ffi_cif* aCif, void* aRet, void** aArgs, void* aUserData)
{
    (void)aCif;
    (void)aRet;

    const auto* context = static_cast<Context*>(aUserData);

    if (!context || !context->reflection || !context->property)
    {
        return;
    }

    auto* instance = *static_cast<Red::IScriptable**>(aArgs[0]);
    auto* stackFrame = *static_cast<Red::CStackFrame**>(aArgs[1]);
    auto* out = *static_cast<void**>(aArgs[2]);

    if (!instance || !stackFrame || !out)
    {
        return;
    }

    stackFrame->code++;

    const auto* record = reinterpret_cast<Red::ScriptableTweakDBRecord*>(instance);

    if (const auto* value = context->reflection->GetScriptablePropertyValue(record, context->property))
    {
        context->property->type->Assign(out, value);
    }
}
} // namespace Red
