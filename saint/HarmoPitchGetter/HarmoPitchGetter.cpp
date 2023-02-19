#include "HarmoPitchGetter.h"
#include "HarmoPitchHelper.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>
#include <optional>

namespace saint {

namespace {
std::vector<int> getTicks(const std::vector<HarmoNoteSpan> &spans) {
  std::vector<int> ticks;
  ticks.reserve(spans.size());
  std::transform(spans.begin(), spans.end(), std::back_inserter(ticks),
                 [](const HarmoNoteSpan &span) { return span.beginTick; });
  return ticks;
}

std::vector<std::optional<PlayedNote>>
getNotes(const std::vector<HarmoNoteSpan> &spans) {
  std::vector<std::optional<PlayedNote>> intervals;
  intervals.reserve(spans.size());
  std::transform(spans.begin(), spans.end(), std::back_inserter(intervals),
                 [](const HarmoNoteSpan &span) { return span.playedNote; });
  return intervals;
}
} // namespace

HarmoPitchGetter::HarmoPitchGetter(const std::vector<HarmoNoteSpan> &spans)
    : _ticks(getTicks(spans)), _intervals(getNotes(spans)) {}

std::optional<float> HarmoPitchGetter::getHarmoInterval(double tick) {
  if (!setIntervalIndex(_ticks, &_index, tick)) {
    return std::nullopt;
  }
  const auto &interval = _intervals[_index];
  if (!interval || !interval->interval) {
    return std::nullopt;
  }
  // For now just a linear intra/extrapolation.
  // Let's try this first and if it's not good enough we'll attempt something
  // more elaborate.
  return *interval->interval;
}

} // namespace saint