#pragma once

#include "HarmoPitchTypes.h"

#include <vector>

namespace saint {
class HarmoPitchGetter {
public:
  HarmoPitchGetter(const std::vector<HarmoNoteSpan> &timeSegments);
  std::optional<float> getHarmoPitch(int tick, float pitch) const;
};
} // namespace saint
