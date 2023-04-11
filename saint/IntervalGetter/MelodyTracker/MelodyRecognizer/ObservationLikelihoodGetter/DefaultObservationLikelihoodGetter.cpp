#include "DefaultObservationLikelihoodGetter.h"

namespace saint {
std::unique_ptr<ObservationLikelihoodGetter>
ObservationLikelihoodGetter::createInstance() {
  return std::make_unique<DefaultObservationLikelihoodGetter>();
}

std::unordered_map<int, float>
DefaultObservationLikelihoodGetter::getObservationLogLikelihoods(
    const std::vector<std::pair<std::chrono::milliseconds, float>>
        &observationSamples) {
  return {};
}
} // namespace saint