#include "DefaultIntervalGetter.h"
#include "IntervalHelper.h"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace saint {

namespace {
std::vector<float> getCrotchets(const std::vector<IntervalSpan> &spans) {
  std::vector<float> crotchets;
  crotchets.reserve(spans.size());
  std::transform(spans.begin(), spans.end(), std::back_inserter(crotchets),
                 [](const IntervalSpan &span) { return span.beginCrotchet; });
  return crotchets;
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

DefaultIntervalGetter::DefaultIntervalGetter(
    const std::vector<IntervalSpan> &spans,
    std::optional<testUtils::IntervalGetterDebugCb> debugCb)
    : _debugCb(std::move(debugCb)), _crotchets(getCrotchets(spans)),
      _intervals(getNotes(spans)) {}

std::optional<float> DefaultIntervalGetter::getHarmoInterval(
    float timeInCrotchets,
    const std::optional<std::function<float(int)>> &getPitchLlh,
    const std::chrono::milliseconds &,
    std::optional<size_t> &melodyReconizerOutput, int blockSize) {
  const auto interval = _getHarmoInterval(timeInCrotchets, getPitchLlh);
  if (_debugCb) {
    testUtils::IntervalGetterDebugCbArgs args{_crotchets, std::nullopt,
                                              interval};
    args.newIndex = _currentIndex;
    args.blockSize = blockSize;
    (*_debugCb)(args);
  }
  return interval;
}

std::optional<float> DefaultIntervalGetter::_getHarmoInterval(
    float timeInCrotchets,
    const std::optional<std::function<float(int)>> &getPitchLlh) {
  if (_prevWasPitched && getPitchLlh.has_value()) {
    return _getInterval();
  }
  _prevWasPitched = getPitchLlh.has_value();
  const auto newIndex = getClosestLimitIndex(_crotchets, timeInCrotchets);
  if (!newIndex.has_value()) {
    return std::nullopt;
  }
  _currentIndex = *newIndex;
  return _getInterval();
}

std::optional<float> DefaultIntervalGetter::_getInterval() const {
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