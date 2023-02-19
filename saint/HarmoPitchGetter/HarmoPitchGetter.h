#pragma once

#include "HarmoPitchTypes.h"

#include <vector>

namespace saint {
class HarmoPitchGetter {
public:
  HarmoPitchGetter(const std::vector<HarmoNoteSpan> &timeSegments);
  std::optional<float> getHarmoInterval(double tick);
  std::optional<float> getHarmoPitch(double tick);

private:
  const std::vector<int> _ticks;
  const std::vector<std::optional<PlayedNote>> _intervals;
  size_t _index = 0;
};
} // namespace saint
