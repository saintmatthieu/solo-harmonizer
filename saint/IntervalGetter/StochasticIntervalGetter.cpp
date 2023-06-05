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
  _prevPitchHadValue = pitch.has_value();
  return _melodyTracker->tick(pitch);
}
} // namespace saint
