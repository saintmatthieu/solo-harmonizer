#include "StochasticIntervalGetter.h"

namespace saint {
namespace {
std::vector<std::pair<float, std::optional<int>>>
toTimedNoteNumbers(const std::vector<IntervalSpan> &spans) {
  std::vector<std::pair<float, std::optional<int>>> timedNoteNumbers;
  for (const auto &span : spans) {
    timedNoteNumbers.emplace_back(
        span.beginCrotchet,
        span.playedNote.has_value()
            ? std::optional<int>{span.playedNote->noteNumber}
            : std::nullopt);
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
  const auto perfNn =
      pitch.has_value()
          ? std::optional<float>{12.f * std::log2f(*pitch / 440.f) + 69.f}
          : std::nullopt;
  const auto warpedTime =
      _perfTimeWarper->getWarpedTime(timeInCrotchets, perfNn);
  if (!perfNn.has_value()) {
    return std::nullopt;
  }
  return _pitchMapper->getHarmony(*perfNn, warpedTime);
}
} // namespace saint
