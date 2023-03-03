#include "IntervalGetter.h"
#include "IntervalHelper.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>
#include <optional>

namespace saint {

namespace {
std::vector<int> getTicks(const std::vector<IntervalSpan> &spans) {
  std::vector<int> ticks;
  ticks.reserve(spans.size());
  std::transform(spans.begin(), spans.end(), std::back_inserter(ticks),
                 [](const IntervalSpan &span) { return span.beginTick; });
  return ticks;
}

std::vector<std::optional<PlayedNote>>
getNotes(const std::vector<IntervalSpan> &spans) {
  std::vector<std::optional<PlayedNote>> intervals;
  intervals.reserve(spans.size());
  std::transform(spans.begin(), spans.end(), std::back_inserter(intervals),
                 [](const IntervalSpan &span) { return span.playedNote; });
  return intervals;
}
} // namespace

IntervalGetter::IntervalGetter(const std::vector<IntervalSpan> &spans,
                               double ticksPerCrotchet)
    : _ticksPerCrotchet(ticksPerCrotchet), _ticks(getTicks(spans)),
      _intervals(getNotes(spans)) {}

std::optional<float>
IntervalGetter::getHarmoInterval(double timeInCrotchets,
                                 const std::optional<float> &pitch) {
  if (_prevWasPitched && pitch.has_value()) {
    // Pitch is stable -> pitch shift interval is locked.
    return _getInterval();
  }
  _prevWasPitched = pitch.has_value();
  const auto tick = timeInCrotchets * _ticksPerCrotchet;
  const auto newIndex = getClosestLimitIndex(_ticks, tick);
  if (!newIndex.has_value()) {
    return std::nullopt;
  }
  _currentIndex = *newIndex;
  return _getInterval();
}

std::optional<float> IntervalGetter::_getInterval() const {
  const auto &interval = _intervals[_currentIndex];
  if (!interval || !interval->interval) {
    return std::nullopt;
  }
  // For now just a linear intra/extrapolation.
  // Let's try this first and if it's not good enough we'll attempt something
  // more elaborate.
  return *interval->interval;
}

} // namespace saint