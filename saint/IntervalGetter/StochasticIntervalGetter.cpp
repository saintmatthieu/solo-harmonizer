#include "StochasticIntervalGetter.h"

namespace saint {
std::unique_ptr<TargetIntervalEstimator>
TargetIntervalEstimator::createInstance(const std::vector<IntervalSpan> &) {
  return nullptr;
}

StochasticIntervalGetter::StochasticIntervalGetter(
    const std::vector<IntervalSpan> &spans,
    const std::map<float, Fraction> &timeSignatures)
    : _spans(spans), _targetSpanIt(_spans.begin()),
      _targetIntervalEstimator(TargetIntervalEstimator::createInstance(_spans)),
      _pitchMapper(PitchMapper::createInstance(_spans, timeSignatures)) {}

std::optional<float> StochasticIntervalGetter::getHarmoInterval(
    float timeInCrotchets, const std::optional<float> &pitch, int blockSize) {
  _targetIntervalEstimator->updateIterator(timeInCrotchets, pitch,
                                           _targetSpanIt);
  if (!pitch.has_value() || _targetSpanIt == _spans.end()) {
    return std::nullopt;
  }
  return _pitchMapper->getHarmony(*pitch, _targetSpanIt);
}
} // namespace saint
