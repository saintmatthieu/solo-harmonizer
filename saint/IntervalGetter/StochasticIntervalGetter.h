#pragma once

#include "CommonTypes.h"
#include "IntervalGetter.h"
#include "MelodyTracker/MelodyTracker.h"
#include "PitchMapper.h"

namespace saint {
class StochasticIntervalGetter : public IntervalGetter {
public:
  StochasticIntervalGetter(const std::vector<IntervalSpan> &spans,
                           std::unique_ptr<MelodyTracker>,
                           const std::map<float, Fraction> &timeSignatures);
  std::optional<float> getHarmoInterval(float timeInCrotchets,
                                        const std::optional<float> &pitch,
                                        const std::chrono::milliseconds &now,
                                        int blockSize = 0) override;

private:
  const std::vector<IntervalSpan> _spans;
  const std::unique_ptr<MelodyTracker> _melodyTracker;
  const std::unique_ptr<PitchMapper> _pitchMapper;
  bool _prevPitchHadValue = false;
  std::optional<float> _nextNoteonTimeEstimate;
  int _tick = 0;
};
} // namespace saint
