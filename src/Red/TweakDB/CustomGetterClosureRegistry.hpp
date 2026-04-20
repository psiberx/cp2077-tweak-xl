#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <vector>

#include <ffi.h>

#include "Red/TweakDB/Manager.hpp"

namespace Red
{
class CustomGetterClosureRegistry
{
public:
    using GetterFn = ScriptingFunction_t<void*>;

    struct Context
    {
        TweakDBManager* manager;
        PropertyInfo property;
    };

    ~CustomGetterClosureRegistry();

    GetterFn CreateGetter(TweakDBManager* aManager, const PropertyInfo& aProperty);

private:
    static void FfiDispatch(ffi_cif* aCif, void* aRet, void** aArgs, void* aUserData);

    struct Entry
    {
        ffi_closure* closure{};
        void* executable{};
        Context context;
    };

    std::mutex m_mutex;
    std::vector<std::unique_ptr<Entry>> m_entries;
    ffi_cif m_cif{};
    std::array<ffi_type*, 4> m_argTypes{{&ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer, &ffi_type_sint64}};
    bool m_cifReady = false;
};
} // namespace Red
