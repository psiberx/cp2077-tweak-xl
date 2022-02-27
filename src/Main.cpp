#include "App/Application.hpp"
#include "Core/Facades/Hook.hpp"
#include "Core/Facades/Runtime.hpp"
#include "Engine/Raws.hpp"

namespace
{
using App::Application;

Core::UniquePtr<Application> g_app;
}

// ASI

BOOL APIENTRY DllMain(HMODULE aHandle, DWORD aReason, LPVOID aReserved)
{
    switch (aReason) // NOLINT(hicpp-multiway-paths-covered)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(aHandle);

        if (Core::Runtime::IsASI(aHandle))
        {
            g_app = Core::MakeUnique<Application>(aHandle);

            Core::Hook::Before<Engine::Raw::Main>(+[] {
                g_app->Bootstrap();
            });
        }
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        if (Core::Runtime::IsASI(aHandle))
        {
            g_app->Shutdown();
            g_app = nullptr;
        }
        break;
    }
    }

    return TRUE;
}

// RED4ext

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason,
                                        const RED4ext::Sdk* aSdk)
{
    switch (aReason)
    {
    case RED4ext::EMainReason::Load:
    {
        g_app = Core::MakeUnique<Application>(aHandle, aSdk);
        g_app->Bootstrap();
        break;
    }
    case RED4ext::EMainReason::Unload:
    {
        g_app->Shutdown();
        g_app = nullptr;
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}
