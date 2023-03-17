#pragma once

#include "CommonTypes.h"
#include "IntervalGetterDebugCb.h"

#include <vector>

namespace saint {
class IntervalGetter {
public:
  IntervalGetter(const std::vector<IntervalSpan> &timeSegments,
                 float ticksPerCrotchet,
                 std::optional<testUtils::IntervalGetterDebugCb>);
  // The caller should only pass a nullopt pitch whenever there is no pitch to
  // be had, or a pitch jump was detected.
  // When `pitch == nullopt`, the algorithm will allow pitch shift jumps.
  std::optional<float> getHarmoInterval(float timeInCrotchets,
                                        const std::optional<float> &pitch,
                                        int blockSize = 0);

private:
  std::optional<float> _getInterval() const;
  std::optional<float> _getHarmoInterval(float timeInCrotchets,
                                         const std::optional<float> &pitch);
  const std::optional<testUtils::IntervalGetterDebugCb> _debugCb;
  const float _ticksPerCrotchet;
  const std::vector<int> _ticks;
  const std::vector<std::optional<PlayedNote>> _intervals;
  bool _prevWasPitched = false;
  int _currentIndex = 0;
};
} // namespace saint
