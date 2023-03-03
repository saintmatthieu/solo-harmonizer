#pragma once

#include "IntervalTypes.h"

#include <vector>

namespace saint {
class IntervalGetter {
public:
  IntervalGetter(const std::vector<IntervalSpan> &timeSegments,
                 double ticksPerCrotchet);
  // The caller should only pass a nullopt pitch whenever there is no pitch to
  // be had, or a pitch jump was detected.
  // When `pitch == nullopt`, the algorithm will allow pitch shift jumps.
  std::optional<float> getHarmoInterval(double timeInCrotchets,
                                        const std::optional<float> &pitch);

private:
  std::optional<float> _getInterval() const;
  const double _ticksPerCrotchet;
  const std::vector<int> _ticks;
  const std::vector<std::optional<PlayedNote>> _intervals;
  bool _prevWasPitched = false;
  int _lastIndex = 0;
};
} // namespace saint
