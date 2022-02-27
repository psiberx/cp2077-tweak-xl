#include "ScriptExtender.hpp"
#include "Engine/Scripting/RTTIRegistrar.hpp"
#include "Scripting/ScriptableTweak.hpp"
#include "Scripting/ScriptedInterface.hpp"
#include "Scripting/ScriptedManager.hpp"

void App::ScriptExtender::Bootstrap()
{
    Engine::RTTIRegistrar::RegisterPending();
}
