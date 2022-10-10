#pragma once

#include "Red/Rtti/Class.hpp"

namespace App
{
class ScriptableTweak : public Red::Rtti::Class<ScriptableTweak>
{
    friend Descriptor;

    inline static void OnRegister(Descriptor* aType)
    {
        aType->SetFlags({ .isAbstract = true });
    }
};
}
