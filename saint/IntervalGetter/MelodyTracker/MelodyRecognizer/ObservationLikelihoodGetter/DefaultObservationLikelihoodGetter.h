#pragma once

#include "ObservationLikelihoodGetter.h"

#include <unordered_set>

namespace saint {
class DefaultObservationLikelihoodGetter : public ObservationLikelihoodGetter {
public:
  DefaultObservationLikelihoodGetter(std::unordered_set<int> intervals);

  std::unordered_map<int, float> getObservationLogLikelihoods(
      const std::vector<std::pair<std::chrono::milliseconds, float>>
          &observationSamples) override;

private:
  const std::unordered_set<int> _intervals;
  const std::unordered_map<int, float> _neutralLikelihoods;
  std::vector<std::pair<std::chrono::milliseconds, float>> _prevObservation;
};
} // namespace saint
