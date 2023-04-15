#include "DefaultObservationLikelihoodGetter.h"
#include "MelodyTracker/MelodyTrackerHelper.h"

#include "Eigen/Eigen"
#include <numeric>

namespace saint {
namespace {
std::unordered_set<int> getIntervalSet(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  const auto intervals =
      MelodyTrackerHelper::getIntervals(MelodyTrackerHelper::getMelody(melody));
  return {intervals.begin(), intervals.end()};
}

std::map<int, float>
getNeutralLikelihoods(const std::unordered_set<int> &intervalSet) {
  std::map<int, float> llhds;
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

std::array<float, 2>
getPitchFitMinMax(const std::vector<std::pair<std::chrono::milliseconds, float>>
                      &observationSamples) {
  // We fit a line in the observed pitches to accommodate bends.
  // Then if there is a bend, we don't know if it's from or to the file
  // interval. We accept both possibilities, and hence take the min/max
  // combination of the two successive notes, to finally take the max
  // probability.
  Eigen::MatrixXf A(observationSamples.size(), 2);
  Eigen::VectorXf B(observationSamples.size());
  for (auto i = 0u; i < observationSamples.size(); ++i) {
    const auto &entry = observationSamples[i];
    const auto time = static_cast<float>(entry.first.count());
    const auto nn = entry.second;
    A(i, 0) = time;
    A(i, 1) = 1.f;
    B(i) = nn;
  }
  const auto At = A.transpose();
  const auto x = (At * A).inverse() * At * B;
  const auto beginTime =
      static_cast<float>(observationSamples[0].first.count());
  const auto endTime = static_cast<float>(
      observationSamples[observationSamples.size() - 1u].first.count());
  const auto beginNn = x[0] * beginTime + x[1u];
  const auto endNn = x[0] * endTime + x[1u];
  const auto min = std::min(beginNn, endNn);
  const auto max = std::max(beginNn, endNn);
  return {min, max};
}

std::map<int, float>
DefaultObservationLikelihoodGetter::getObservationLogLikelihoods(
    const std::vector<std::pair<std::chrono::milliseconds, float>>
        &observationSamples) {
  const auto prevWasEmpty = _prevObservation.empty();
  const auto minMax = getPitchFitMinMax(observationSamples);
  if (prevWasEmpty) {
    _prevObservation = observationSamples;
    _prevMinMax = minMax;
    return _neutralLikelihoods;
  }
  const auto [min, max] = minMax;
  const auto [pmin, pmax] = _prevMinMax;
  _prevMinMax = minMax;
  std::map<int, float> likelihoods;
  const std::array<float, 4> combinations{min - pmin, max - pmin, min - pmax,
                                          max - pmax};
  for (auto interval : _intervals) {
    std::array<float, 4> possibilities;
    std::transform(combinations.begin(), combinations.end(),
                   possibilities.begin(), [interval](float measuredInterval) {
                     // If we use a normal distribution as shape for our
                     // "probability", the log
                     // of it is just a linear function in the squared error
                     // Not sure it's useful, but it's cheap anyway, to use
                     // this function. Let's say our standard deviation is
                     // 0.5, meaning there's a 95% chance that an intended
                     // interval is within +/- 1 semitone of its intended
                     // value. Then the a and b coefficients of this function
                     // are a = 1/(2*std^2) b =
                     // -Math.log(std*Math.sqrt(2*Math.PI)) i.e., (evaluated
                     // in node)
                     constexpr auto a = -2.f;
                     constexpr auto b = -0.22579135264472733f;
                     const auto err = measuredInterval - interval;
                     return a * err * err + b;
                   });
    likelihoods[interval] =
        *std::max_element(possibilities.begin(), possibilities.end());
  }
  return likelihoods;
}
} // namespace saint