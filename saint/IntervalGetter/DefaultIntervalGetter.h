#pragma once

#include "CommonTypes.h"
#include "IntervalGetter.h"
#include "IntervalGetterDebugCb.h"

#include <chrono>
#include <vector>

namespace saint {
using namespace std::literals::chrono_literals;

class DefaultIntervalGetter : public IntervalGetter {
public:
  DefaultIntervalGetter(const std::vector<IntervalSpan> &timeSegments,
                        std::optional<testUtils::IntervalGetterDebugCb>);
  std::optional<float>
  getHarmoInterval(float timeInCrotchets, const std::optional<float> &pitch,
                   float pitchConfidence, const std::chrono::milliseconds &now = 0ms,
                   int blockSize = 0) override;

private:
  std::optional<float> _getInterval() const;
  std::optional<float> _getHarmoInterval(float timeInCrotchets,
                                         const std::optional<float> &pitch,
                                         float pitchConfidence);
  const std::optional<testUtils::IntervalGetterDebugCb> _debugCb;
  const std::vector<float> _crotchets;
  const std::vector<std::optional<PlayedNote>> _intervals;
  bool _prevWasPitched = false;
  int _currentIndex = 0;
};
} // namespace saint
