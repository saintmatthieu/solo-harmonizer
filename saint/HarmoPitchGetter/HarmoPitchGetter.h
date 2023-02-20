#pragma once

#include "HarmoPitchTypes.h"

#include <vector>

namespace saint {
class HarmoPitchGetter {
public:
  HarmoPitchGetter(const std::vector<HarmoNoteSpan> &timeSegments,
                   double ticksPerCrotchet);
  std::optional<float> getHarmoInterval(double timeInCrotchets);

private:
  const double _ticksPerCrotchet;
  const std::vector<int> _ticks;
  const std::vector<std::optional<PlayedNote>> _intervals;
  size_t _index = 0;
};
} // namespace saint
