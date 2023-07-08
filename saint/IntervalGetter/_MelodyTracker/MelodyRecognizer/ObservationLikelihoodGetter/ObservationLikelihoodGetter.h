#pragma once

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace saint {
class ObservationLikelihoodGetter {
public:
  static std::unique_ptr<ObservationLikelihoodGetter> createInstance(
      const std::vector<std::pair<float, std::optional<int>>> &melody);

  virtual std::map<int, float> getObservationLogLikelihoods(
      const std::vector<std::pair<std::chrono::milliseconds, float>>
          &observationSamples) = 0;

  virtual ~ObservationLikelihoodGetter() = default;
};

} // namespace saint