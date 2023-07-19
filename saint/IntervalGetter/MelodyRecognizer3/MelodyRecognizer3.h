#pragma once

#include <functional>
#include <optional>
#include <vector>

namespace saint {
class MelodyRecognizer3 {
public:
  using Melody = std::vector<std::pair<float, std::optional<int>>>;
  MelodyRecognizer3(Melody melody,
                    std::optional<float> observationLikelihoodWeight);
  std::optional<size_t>
  tick(const std::optional<std::function<float(float)>> &getPitchLikelihood,
       std::optional<size_t> &guessedState);

public:
  struct Params {
    const float llhThatItStays;
    const float transitionToNextLlh;
    const float observationLlhWeight;
  };

private:
  float _getTransitionLikelihood(size_t oldState, size_t newState,
                                 const Params &) const;
  std::vector<float> _getObservationLikelihoods(
      const std::function<float(int)> &getPitchLikelihood) const;
  const Melody _melody;
  const float _observationLikelihoodWeight;
  const std::vector<size_t> _stateToMelodyIndices;
  std::vector<float> _priors;
  std::vector<float> _newPriors;
  size_t _stateCount =
      0u; // num. of consecutive times the `tick` input isn't nullopt.
  size_t _tickCount = 0u; // for debugging
  size_t _winnerIndex = -1;
  bool _prevReturnHadValue = false;
};
} // namespace saint
