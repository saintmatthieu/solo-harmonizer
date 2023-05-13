#pragma once

#include <map>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace saint {
class MelodyRecognizer2 {
public:
  using Melody =
      std::vector<std::pair<float /*duration*/, int /*note number*/>>;
  struct MotiveInstance {
    size_t beginIndex;
    int firstNoteNumber;
    float firstDuration;
  };

  MelodyRecognizer2(Melody melody);
  std::optional<size_t> onNoteOff(const std::vector<float> &noteNumbers);

private:
  const float _referenceDuration;
  // Could be a map, i.e., std::map<std::vector<std::pair<float, int>>,
  // std::set<size_t>>
  const std::vector<std::pair<std::vector<std::pair<float, int>> /*motive*/,
                              std::vector<MotiveInstance>>>
      _motiveInstances;
  std::vector<std::vector<float>> _lastExperiments;
  std::optional<size_t> _lastGuess;
};
} // namespace saint
