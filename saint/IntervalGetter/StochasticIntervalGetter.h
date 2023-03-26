#pragma once

#include "CommonTypes.h"
#include "IntervalGetter.h"
#include "PerformanceTimeWarper.h"
#include "PitchMapper.h"

namespace saint {
class StochasticIntervalGetter : public IntervalGetter {
public:
  StochasticIntervalGetter(const std::vector<IntervalSpan> &spans,
                           const std::map<float, Fraction> &timeSignatures);
  std::optional<float> getHarmoInterval(float timeInCrotchets,
                                        const std::optional<float> &pitch,
                                        int blockSize = 0) override;

private:
  const std::vector<IntervalSpan> _spans;
  const std::unique_ptr<PerformanceTimeWarper> _perfTimeWarper;
  const std::unique_ptr<PitchMapper> _pitchMapper;
};
} // namespace saint
