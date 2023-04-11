#include "StochasticIntervalGetter.h"

namespace saint {
StochasticIntervalGetter::StochasticIntervalGetter(
    const std::vector<IntervalSpan> &spans,
    std::unique_ptr<MelodyTracker> melodyTracker, std::unique_ptr<Clock> clock,
    const std::map<float, Fraction> &timeSignatures)
    : _spans(spans), _melodyTracker(std::move(melodyTracker)),
      _pitchMapper(PitchMapper::createInstance(_spans, timeSignatures)),
      _clock(std::move(clock)) {}

std::optional<float> StochasticIntervalGetter::getHarmoInterval(
    float timeInCrotchets, const std::optional<float> &pitch, int blockSize) {
  const auto perfNn =
      pitch.has_value()
          ? std::optional<float>{12.f * std::log2f(*pitch / 440.f) + 69.f}
          : std::nullopt;
  if (pitch.has_value()) {
    const auto noteIndex =
        _melodyTracker->onNoteOnSample(_clock->now(), *perfNn);
    const auto spanTime = _spans[noteIndex].beginCrotchet;
    return _pitchMapper->getHarmony(*perfNn, spanTime);
  } else {
    if (_prevPitchHadValue) {
      _melodyTracker->onNoteOff();
    }
    return std::nullopt;
  }
}
} // namespace saint
