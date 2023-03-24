#pragma once

#include "CommonTypes.h"
#include "IntervalGetter.h"
#include "PitchMapper.h"

namespace saint {
class TargetIntervalEstimator {
public:
  static std::unique_ptr<TargetIntervalEstimator>
  createInstance(const std::vector<IntervalSpan> &);

  virtual void updateIterator(float timeInCrotchets,
                              const std::optional<float> &pitch,
                              std::vector<IntervalSpan>::const_iterator &) = 0;

  virtual ~TargetIntervalEstimator() = default;
};

class StochasticIntervalGetter : public IntervalGetter {
public:
  StochasticIntervalGetter(const std::vector<IntervalSpan> &spans,
                           const std::map<float, Fraction> &timeSignatures);
  std::optional<float> getHarmoInterval(float timeInCrotchets,
                                        const std::optional<float> &pitch,
                                        int blockSize = 0) override;

private:
  const std::vector<IntervalSpan> _spans;
  std::vector<IntervalSpan>::const_iterator _targetSpanIt;
  const std::unique_ptr<TargetIntervalEstimator> _targetIntervalEstimator;
  const std::unique_ptr<PitchMapper> _pitchMapper;
};
} // namespace saint
