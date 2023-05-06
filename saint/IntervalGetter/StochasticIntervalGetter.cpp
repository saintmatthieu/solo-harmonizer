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
  if (pitch.has_value()) {
    _melodyTracker->onNoteOnSample(now, *perfNn);
    _prevPitchHadValue = true;
    return _nextNoteonTimeEstimate.has_value()
               ? _pitchMapper->getHarmony(*perfNn, *_nextNoteonTimeEstimate)
               : std::nullopt;
  } else {
    if (_prevPitchHadValue) {
      if (const auto nextNoteIndex = _melodyTracker->onNoteOff()) {
        _nextNoteonTimeEstimate = _spans[*nextNoteIndex].beginCrotchet;
      } else {
        _nextNoteonTimeEstimate.reset();
      }
    }
    _prevPitchHadValue = false;
    return std::nullopt;
  }
}
} // namespace saint
