#pragma once

#include "App/Project.hpp"

namespace App
{
class Facade : public Red::IScriptable
{
public:
    static bool RegisterDir(Red::CString& aPath);
    static bool RegisterTweak(Red::CString& aPath);
    static void ImportAll();
    static void ImportDir(Red::CString& aPath);
    static void ImportTweak(Red::CString& aPath);
    static void ExecuteAll();
    static void ExecuteTweak(Red::CName aName);
    static void Reload();
    static bool Require(Red::CString& aVersion);
    static Red::CString GetVersion();

    RTTI_IMPL_TYPEINFO(App::Facade);
};
}

RTTI_DEFINE_CLASS(App::Facade, App::Project::Name, {
    RTTI_ABSTRACT();
    RTTI_METHOD(RegisterDir);
    RTTI_METHOD(RegisterTweak);
    RTTI_METHOD(ImportAll);
    RTTI_METHOD(ImportDir);
    RTTI_METHOD(ImportTweak, "Import");
    RTTI_METHOD(ExecuteAll);
    RTTI_METHOD(ExecuteTweak, "Execute");
    RTTI_METHOD(Reload);
    RTTI_METHOD(Require);
    RTTI_METHOD(GetVersion, "Version");
})
