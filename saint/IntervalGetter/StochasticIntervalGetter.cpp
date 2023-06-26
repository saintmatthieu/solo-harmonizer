#include "StochasticIntervalGetter.h"

#include <cassert>

namespace saint {
StochasticIntervalGetter::StochasticIntervalGetter(
    const std::vector<IntervalSpan> &spans,
    std::unique_ptr<MelodyTracker> melodyTracker,
    const std::map<float, Fraction> &timeSignatures)
    : _spans(spans), _melodyTracker(std::move(melodyTracker)),
      _pitchMapper(PitchMapper::createInstance(_spans, timeSignatures)) {}

std::optional<float> StochasticIntervalGetter::getHarmoInterval(
    float timeInCrotchets, const std::optional<float> &pitch,
    float pitchConfidence, const std::chrono::milliseconds &now,
    int blockSize) {
  const auto perfNn =
      pitch.has_value()
          ? std::optional<float>{12.f * std::log2f(*pitch / 440.f) + 69.f}
          : std::nullopt;
  const auto tick = _tick++;
  _prevPitchHadValue = pitch.has_value();
  const auto guessedSpanIndex = _melodyTracker->tick(perfNn, pitchConfidence);
  if (!guessedSpanIndex.has_value() || *guessedSpanIndex >= _spans.size()) {
    return std::nullopt;
  }
  const auto &playedNote = _spans[*guessedSpanIndex].playedNote;
  if (!playedNote.has_value() || !playedNote->interval.has_value()) {
    return std::nullopt;
  }
  return *playedNote->interval;
}
} // namespace saint
