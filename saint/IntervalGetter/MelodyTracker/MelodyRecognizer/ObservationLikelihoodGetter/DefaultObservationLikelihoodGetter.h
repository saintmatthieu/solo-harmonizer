#pragma once

#include "ObservationLikelihoodGetter.h"

#include <array>
#include <map>
#include <unordered_set>

namespace saint {
class DefaultObservationLikelihoodGetter : public ObservationLikelihoodGetter {
public:
  DefaultObservationLikelihoodGetter(std::unordered_set<int> intervals);

  std::map<int, float> getObservationLogLikelihoods(
      const std::vector<std::pair<std::chrono::milliseconds, float>>
          &observationSamples) override;

private:
  const std::unordered_set<int> _intervals;
  const std::map<int, float> _neutralLikelihoods;
  std::vector<std::pair<std::chrono::milliseconds, float>> _prevObservation;
  std::array<float, 2> _prevMinMax;
};
} // namespace saint
