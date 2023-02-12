#pragma once

#include "../CommonTypes.h"
#include "HarmoPitchTypes.h"

#include <vector>

namespace saint {
std::vector<HarmoNoteSpan>
toHarmoPitchGetterInput(const Config &, int *ticksPerCrotchet = nullptr);
}