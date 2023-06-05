#pragma once

#include <optional>
#include <vector>

namespace saint {
class MelodyRecognizer3 {
public:
  using Melody = std::vector<std::pair<float, std::optional<int>>>;
  MelodyRecognizer3(Melody melody);
  std::optional<size_t> tick(const std::optional<float> &measuredNoteNumber);

private:
  const Melody _melody;
  const size_t _numStates;
  const std::vector<std::vector<float>> _transitionMatrix;
  std::vector<float> _priors;
  std::vector<float> _newPriors;
};
} // namespace saint
