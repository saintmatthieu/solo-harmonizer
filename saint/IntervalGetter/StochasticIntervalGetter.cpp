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
    float timeInCrotchets,
    const std::optional<std::function<float(int)>> &getPitchLlh,
    const std::chrono::milliseconds &now, int blockSize) {
  const auto tick = _tick++;
  _prevPitchHadValue = getPitchLlh.has_value();
  const auto guessedSpanIndex = _melodyTracker->tick(getPitchLlh);
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
