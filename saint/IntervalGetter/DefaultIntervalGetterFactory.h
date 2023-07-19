#pragma once

#include "IntervalGetterFactory.h"

namespace saint {
class DefaultIntervalGetterFactory : public IntervalGetterFactory {
public:
  DefaultIntervalGetterFactory(
      std::optional<float> observationLikelihoodWeight);
  std::shared_ptr<IntervalGetter>
  createInstance(const std::vector<IntervalSpan> &spans,
                 const std::map<float, Fraction> &timeSignatures,
                 const std::optional<int> &samplesPerSecond,
                 const std::optional<float> &crotchetsPerSecond) const override;

private:
  const std::optional<float> _observationLikelihoodWeight;
};
} // namespace saint