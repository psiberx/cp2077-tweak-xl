#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <RED4ext/CName.hpp>
#include <RED4ext/CString.hpp>
#include <RED4ext/DynArray.hpp>
#include <RED4ext/Handle.hpp>
#include <RED4ext/HashMap.hpp>
#include <RED4ext/NativeTypes.hpp>
#include <RED4ext/ResourcePath.hpp>
#include <RED4ext/ResourceReference.hpp>
#include <RED4ext/SortedArray.hpp>
#include <RED4ext/TweakDB.hpp>
#include <RED4ext/Scripting/Natives/Generated/Color.hpp>
#include <RED4ext/Scripting/Natives/Generated/EulerAngles.hpp>
#include <RED4ext/Scripting/Natives/Generated/Vector2.hpp>
#include <RED4ext/Scripting/Natives/Generated/Vector3.hpp>
#include <RED4ext/Scripting/Natives/Generated/Quaternion.hpp>

#include <RED4ext/Api/EMainReason.hpp>
#include <RED4ext/Api/Sdk.hpp>
#include <RED4ext/Api/Runtime.hpp>
#include <RED4ext/Api/SemVer.hpp>
#include <RED4ext/Api/Version.hpp>

#include "Core/Raw.hpp"
#include "Core/Stl.hpp"
#include "Engine/Stl.hpp"

#ifdef VERBOSE
#include "Engine/Log.hpp"
#endif
