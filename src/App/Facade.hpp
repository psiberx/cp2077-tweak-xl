#pragma once

#include "Red/Rtti/Class.hpp"

namespace App
{
class Facade : public Red::Rtti::Class<Facade>
{
public:
    static void Reload();
    static void ImportAll();
    static void ImportDir(RED4ext::CString& aPath);
    static void ImportTweak(RED4ext::CString& aPath);
    static void ExecuteAll();
    static void ExecuteTweak(RED4ext::CName aName);
    static RED4ext::CString GetVersion();

private:
    friend Descriptor;
    static void OnRegister(Descriptor* aType);
    static void OnDescribe(Descriptor* aType, RED4ext::CRTTISystem* aRtti);
};
}
