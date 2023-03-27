#include "DefaultPerformanceTimeWarper.h"
#include "IntervalHelper.h"
#include "PerformanceTimeWarperDebugCb.h"

#include <algorithm>
#include <cassert>

namespace saint {
std::unique_ptr<PerformanceTimeWarper> PerformanceTimeWarper::createInstance(
    const std::map<float, std::optional<int>> &timedNoteNumbers) {
  return std::make_unique<DefaultPerformanceTimeWarper>(timedNoteNumbers);
}

namespace {
std::vector<float> getNoteBeginCrotchets(
    const std::map<float, std::optional<int>> &timedNoteNumbers) {
  std::vector<float> crotchets(timedNoteNumbers.size());
  std::transform(timedNoteNumbers.begin(), timedNoteNumbers.end(),
                 crotchets.begin(),
                 [](const auto &entry) { return entry.first; });
  return crotchets;
}
} // namespace

DefaultPerformanceTimeWarper::DefaultPerformanceTimeWarper(
    const std::map<float, std::optional<int>> &timedNoteNumbers)
    : _noteBeginCrotchets(getNoteBeginCrotchets(timedNoteNumbers)) {}

float DefaultPerformanceTimeWarper::getWarpedTime(
    float timeInCrotchets, const std::optional<float> &pitch) {
  // For now we re-use the DefaultIntervalGetter implementation, but the plan is
  // to use stochastics.
  const auto index = getClosestLimitIndex(_noteBeginCrotchets, timeInCrotchets);
  const auto warpedTime = index.has_value() ? _noteBeginCrotchets[*index]
                                            : _noteBeginCrotchets[_lastIndex];
  if (index.has_value()) {
    _lastIndex = *index;
  }
  if (auto debugCb = testUtils::getPerformanceTimeWarperDebugCb()) {
    (*debugCb)(timeInCrotchets, pitch, warpedTime);
  }
  return warpedTime;
}
} // namespace saint