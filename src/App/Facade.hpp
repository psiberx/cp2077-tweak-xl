#pragma once

#include "Red/Rtti/Class.hpp"

namespace App
{
class Facade : public Red::Rtti::Class<Facade>
{
public:
    static void Reload();
    static void ImportAll();
    static void ImportDir(Red::CString& aPath);
    static void ImportTweak(Red::CString& aPath);
    static void ExecuteAll();
    static void ExecuteTweak(Red::CName aName);
    static Red::CString GetVersion();

private:
    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnDescribe(Descriptor* aType);
};
}
