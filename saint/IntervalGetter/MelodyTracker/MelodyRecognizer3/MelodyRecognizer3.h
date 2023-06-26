#pragma once

#include <optional>
#include <vector>

namespace saint {
class MelodyRecognizer3 {
public:
  using Melody = std::vector<std::pair<float, std::optional<int>>>;
  MelodyRecognizer3(Melody melody);
  std::optional<size_t> tick(const std::optional<float> &measuredNoteNumber,
                             float pitchConfidence);

private:
  float _getTransitionLikelihood(size_t oldState, size_t newState) const;
  std::vector<float> _getObservationLikelihoods(float measuredNoteNumber,
                                                float pitchConfidence) const;
  const Melody _melody;
  const std::vector<size_t> _stateToMelodyIndices;
  std::vector<float> _priors;
  std::vector<float> _newPriors;
  size_t _stateCount =
      0u; // num. of consecutive times the `tick` input isn't nullopt.
  size_t _tickCount = 0u; // for debugging
  size_t _winnerIndex = -1;
};
} // namespace saint
