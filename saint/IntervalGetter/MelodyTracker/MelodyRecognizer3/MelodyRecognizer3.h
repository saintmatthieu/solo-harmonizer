#pragma once

#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace saint {
class MelodyRecognizer3 {
public:
  using Melody = std::vector<std::pair<float, std::optional<int>>>;
  MelodyRecognizer3(const Melody &melody);
  std::optional<size_t> tick(const std::optional<float> &detectedPitch);

private:
  const size_t _numStates;
  const std::unordered_map<size_t, std::unordered_map<size_t, float>>
      _transitionTable;
  std::unordered_map<size_t, float> _priors;
};
} // namespace saint
