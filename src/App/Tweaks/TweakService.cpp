#include "TweakService.hpp"
#include "App/Tweaks/Declarative/TweakImporter.hpp"
#include "App/Tweaks/Executable/TweakExecutor.hpp"
#include "Red/TweakDB/Manager.hpp"

App::TweakService::TweakService(std::filesystem::path mTweakDir)
    : m_tweakDir(std::move(mTweakDir))
{
}

void App::TweakService::LoadAll()
{
    Red::TweakDB::Manager manager;
    TweakImporter(manager, m_tweakDir).ImportAll();
    TweakExecutor(manager).ExecuteAll();
}

void App::TweakService::ImportAll()
{
    Red::TweakDB::Manager manager;
    TweakImporter(manager, m_tweakDir).ImportAll();
}

void App::TweakService::ImportDir(const std::filesystem::path& aPath)
{
    Red::TweakDB::Manager manager;
    TweakImporter(manager, m_tweakDir).ImportDir(aPath);
}

void App::TweakService::ImportTweak(const std::filesystem::path& aPath)
{
    Red::TweakDB::Manager manager;
    TweakImporter(manager, m_tweakDir).Import(aPath);
}

void App::TweakService::ExecuteAll()
{
    Red::TweakDB::Manager manager;
    TweakExecutor(manager).ExecuteAll();
}

void App::TweakService::ExecuteTweak(RED4ext::CName aName)
{
    Red::TweakDB::Manager manager;
    TweakExecutor(manager).Execute(aName);
}
