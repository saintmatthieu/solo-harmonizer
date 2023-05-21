#include "StochasticIntervalGetter.h"

namespace saint {
StochasticIntervalGetter::StochasticIntervalGetter(
    const std::vector<IntervalSpan> &spans,
    std::unique_ptr<MelodyTracker> melodyTracker,
    const std::map<float, Fraction> &timeSignatures)
    : _spans(spans), _melodyTracker(std::move(melodyTracker)),
      _pitchMapper(PitchMapper::createInstance(_spans, timeSignatures)) {}

std::optional<float> StochasticIntervalGetter::getHarmoInterval(
    float timeInCrotchets, const std::optional<float> &pitch,
    const std::chrono::milliseconds &now, int blockSize) {
  const auto perfNn =
      pitch.has_value()
          ? std::optional<float>{12.f * std::log2f(*pitch / 440.f) + 69.f}
          : std::nullopt;
  const auto tick = _tick++;
  if (pitch.has_value()) {
    if (!_prevPitchHadValue) {
      _nextNoteonTimeEstimate = _melodyTracker->beginNewNote(tick);
    }
    _melodyTracker->addPitchMeasurement(*perfNn);
    _prevPitchHadValue = true;
    return _nextNoteonTimeEstimate.has_value()
               ? _pitchMapper->getHarmony(*perfNn, *_nextNoteonTimeEstimate)
               : std::nullopt;
  }
  _prevPitchHadValue = pitch.has_value();
  return std::nullopt;
}
} // namespace saint
