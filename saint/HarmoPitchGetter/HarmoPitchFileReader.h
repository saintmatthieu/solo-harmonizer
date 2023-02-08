#pragma once

#include "HarmoPitchTypes.h"

#include <filesystem>

namespace saint {
std::vector<HarmoNoteSpan>
toHarmoPitchGetterInput(const std::filesystem::path &xmlConfig);
}