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

IntervalGetter::IntervalGetter(
    const std::vector<IntervalSpan> &spans, float ticksPerCrotchet,
    std::optional<testUtils::IntervalGetterDebugCb> debugCb)
    : _debugCb(std::move(debugCb)), _ticksPerCrotchet(ticksPerCrotchet),
      _ticks(getTicks(spans)), _intervals(getNotes(spans)) {}

std::optional<float> IntervalGetter::getHarmoInterval(
    float timeInCrotchets, const std::optional<float> &pitch, int blockSize) {
  const auto interval = _getHarmoInterval(timeInCrotchets, pitch);
  if (_debugCb) {
    testUtils::IntervalGetterDebugCbArgs args{_ticks, pitch, interval};
    args.newIndex = _currentIndex;
    args.blockSize = blockSize;
    (*_debugCb)(args);
  }
  return interval;
}

std::optional<float>
IntervalGetter::_getHarmoInterval(float timeInCrotchets,
                                  const std::optional<float> &pitch) {
  if (_prevWasPitched || !pitch.has_value()) {
    _prevWasPitched = pitch.has_value();
    // We only are interested in interval changes when the state goes from
    // unpitched to pitched.
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