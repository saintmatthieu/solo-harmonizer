#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace saint {
class ObservationLikelihoodGetter {
public:
  static std::unique_ptr<ObservationLikelihoodGetter> createInstance();

  virtual std::unordered_map<int, float> getObservationLogLikelihoods(
      const std::vector<std::pair<std::chrono::milliseconds, float>>
          &observationSamples) = 0;

  virtual ~ObservationLikelihoodGetter() = default;
};

} // namespace saint