#include "StochasticIntervalGetter.h"

namespace saint {
namespace {
std::map<float, std::optional<int>>
toTimedNoteNumbers(const std::vector<IntervalSpan> &spans) {
  std::map<float, std::optional<int>> timedNoteNumbers;
  for (const auto &span : spans) {
    timedNoteNumbers[span.beginCrotchet] =
        span.playedNote.has_value()
            ? std::optional<int>{span.playedNote->noteNumber}
            : std::nullopt;
  }
  return timedNoteNumbers;
}
} // namespace

StochasticIntervalGetter::StochasticIntervalGetter(
    const std::vector<IntervalSpan> &spans,
    const std::map<float, Fraction> &timeSignatures)
    : _spans(spans), _perfTimeWarper(PerformanceTimeWarper::createInstance(
                         toTimedNoteNumbers(spans))),
      _pitchMapper(PitchMapper::createInstance(_spans, timeSignatures)) {}

std::optional<float> StochasticIntervalGetter::getHarmoInterval(
    float timeInCrotchets, const std::optional<float> &pitch, int blockSize) {
  const auto warpedTime =
      _perfTimeWarper->getWarpedTime(timeInCrotchets, pitch);
  if (!pitch.has_value()) {
    return std::nullopt;
  }
  return _pitchMapper->getHarmony(*pitch, warpedTime);
}
} // namespace saint
