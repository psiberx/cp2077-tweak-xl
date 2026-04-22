#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <vector>

#include <ffi.h>

#include "Red/TweakDB/Types.hpp"

namespace Red
{
class TweakDBReflection;

class ScriptableRecordClosureRegistry
{
public:
    using GetterFn = ScriptingFunction_t<void*>;

    struct Context
    {
        TweakDBReflection* reflection;
        PropertyInfo property;
    };

    ~ScriptableRecordClosureRegistry();

    GetterFn CreateClosure(TweakDBReflection* aReflection, const PropertyInfo& aProperty);
    bool DestroyClosure(GetterFn aClosure);

private:
    static void FfiDispatch(ffi_cif* aCif, void* aRet, void** aArgs, void* aUserData);

    struct Entry
    {
        ffi_closure* closure{};
        void* executable{};
        Context context;
    };

    std::mutex m_mutex;
    Core::Vector<Core::UniquePtr<Entry>> m_entries;
    ffi_cif m_cif{};
    std::array<ffi_type*, 4> m_argTypes{{&ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, &ffi_type_sint64}};
    bool m_cifReady = false;
};
} // namespace Red
