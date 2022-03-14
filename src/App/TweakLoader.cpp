#include "TweakLoader.hpp"
#include "Configuration.hpp"
#include "Import/TweakImporter.hpp"
#include "Scripting/TweakExecutor.hpp"
#include "TweakDB/Manager.hpp"
#include "TweakDB/Raws.hpp"

void App::TweakLoader::OnBootstrap()
{
    HookAfter<TweakDB::Raw::LoadTweakDB>(&OnLoadTweakDB);
}

void App::TweakLoader::OnLoadTweakDB(RED4ext::TweakDB* aTweakDB, RED4ext::CString& aPath)
{
    // Create shared manager
    TweakDB::Manager manager(aTweakDB);

    // Import all static tweaks
    TweakImporter(manager, Configuration::GetTweaksDir()).ImportAll();

    // Run all scriptable tweaks
    TweakExecutor(manager).ExecuteAll();
}
