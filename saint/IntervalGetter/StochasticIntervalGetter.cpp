#include "StochasticIntervalGetter.h"
#include "MelodyRecognizer3/MelodyRecognizer3.h"

#include <cassert>

namespace saint {
StochasticIntervalGetter::StochasticIntervalGetter(
    const std::vector<IntervalSpan> &spans,
    std::unique_ptr<MelodyRecognizer3> melodyRecognizer,
    const std::map<float, Fraction> &timeSignatures)
    : _spans(spans), _melodyRecognizer(std::move(melodyRecognizer)),
      _pitchMapper(PitchMapper::createInstance(_spans, timeSignatures)) {}

std::optional<float> StochasticIntervalGetter::getHarmoInterval(
    float timeInCrotchets,
    const std::optional<std::function<float(int)>> &getPitchLlh,
    const std::chrono::milliseconds &now, int blockSize) {
  const auto tick = _tick++;
  _prevPitchHadValue = getPitchLlh.has_value();
  const auto guessedSpanIndex = _melodyRecognizer->tick(getPitchLlh);
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
