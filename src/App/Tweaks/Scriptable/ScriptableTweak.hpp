#pragma once

#include "Engine/Scripting/RTTIClass.hpp"

namespace App
{
class ScriptableTweak : public Engine::RTTIClass<ScriptableTweak>
{
    friend Descriptor;

    inline static void OnRegister(Descriptor* aType)
    {
        aType->SetFlags({ .isAbstract = true });
    }
};
}
