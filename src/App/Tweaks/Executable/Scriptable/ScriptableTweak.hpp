#pragma once

namespace App
{
class ScriptableTweak : public Red::IScriptable
{
    RTTI_IMPL_TYPEINFO(App::ScriptableTweak);
};
}

RTTI_DEFINE_CLASS(App::ScriptableTweak, {
    RTTI_ABSTRACT();
})
