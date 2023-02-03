#pragma once

#include "HarmoPitchTypes.h"

#include <filesystem>

namespace saint {
std::vector<InputTimeEntry>
toHarmoPitchGetterInput(const std::filesystem::path &xmlConfig);
}