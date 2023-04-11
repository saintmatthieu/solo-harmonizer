#include "DefaultObservationLikelihoodGetter.h"
#include "MelodyTracker/MelodyTrackerHelper.h"

namespace saint {
namespace {
std::unordered_set<int> getIntervalSet(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  const auto intervals =
      MelodyTrackerHelper::getIntervals(MelodyTrackerHelper::getMelody(melody));
  return {intervals.begin(), intervals.end()};
}

std::unordered_map<int, float>
getNeutralLikelihoods(const std::unordered_set<int> &intervalSet) {
  std::unordered_map<int, float> llhds;
  const auto likelihood =
      std::logf(1.f / static_cast<float>(intervalSet.size()));
  for (auto interval : intervalSet) {
    llhds[interval] = likelihood;
  }
  return llhds;
}
} // namespace

DefaultObservationLikelihoodGetter::DefaultObservationLikelihoodGetter(
    std::unordered_set<int> intervals)
    : _intervals(std::move(intervals)),
      _neutralLikelihoods(getNeutralLikelihoods(_intervals)) {}

std::unique_ptr<ObservationLikelihoodGetter>
ObservationLikelihoodGetter::createInstance(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  return std::make_unique<DefaultObservationLikelihoodGetter>(
      getIntervalSet(melody));
}

std::unordered_map<int, float>
DefaultObservationLikelihoodGetter::getObservationLogLikelihoods(
    const std::vector<std::pair<std::chrono::milliseconds, float>>
        &observationSamples) {
  const auto prevWasEmpty = _prevObservation.empty();
  if (prevWasEmpty) {
    _prevObservation = observationSamples;
    return _neutralLikelihoods;
  }

  return {};
}
} // namespace saint