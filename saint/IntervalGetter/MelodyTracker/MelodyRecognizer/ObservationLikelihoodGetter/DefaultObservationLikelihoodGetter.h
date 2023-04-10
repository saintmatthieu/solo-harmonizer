#pragma once

#include "ObservationLikelihoodGetter.h"

namespace saint {
class DefaultObservationLikelihoodGetter : public ObservationLikelihoodGetter {
public:
  std::unordered_map<int, float> getObservationLogLikelihoods(
      const std::vector<std::pair<std::chrono::milliseconds, float>>
          &observationSamples) override;
};
} // namespace saint
