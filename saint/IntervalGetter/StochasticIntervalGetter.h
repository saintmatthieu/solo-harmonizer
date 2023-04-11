#pragma once

#include "CommonTypes.h"
#include "IntervalGetter.h"
#include "MelodyTracker/MelodyTracker.h"
#include "PitchMapper.h"

namespace saint {
class Clock {
public:
  virtual std::chrono::milliseconds now() const = 0;
  virtual ~Clock() = default;
};

class StochasticIntervalGetter : public IntervalGetter {
public:
  StochasticIntervalGetter(const std::vector<IntervalSpan> &spans,
                           std::unique_ptr<MelodyTracker>,
                           std::unique_ptr<Clock>,
                           const std::map<float, Fraction> &timeSignatures);
  std::optional<float> getHarmoInterval(float timeInCrotchets,
                                        const std::optional<float> &pitch,
                                        int blockSize = 0) override;

private:
  const std::vector<IntervalSpan> _spans;
  const std::unique_ptr<MelodyTracker> _melodyTracker;
  const std::unique_ptr<PitchMapper> _pitchMapper;
  const std::unique_ptr<Clock> _clock;
  bool _prevPitchHadValue = false;
};
} // namespace saint
