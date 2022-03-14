#pragma once

#include "Core/Foundation/Feature.hpp"

namespace App
{
class ScriptExtender : public Core::Feature
{
    void OnBootstrap() override;
};
}
