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
  std::optional<size_t> beginNewNote(int tickCounter);
  void addPitchMeasurement(float pc);

private:
  const float _referenceDuration;
  // Could be a map, i.e., std::map<std::vector<std::pair<float, int>>,
  // std::set<size_t>>
  const std::vector<std::pair<std::vector<std::pair<float, int>> /*motive*/,
                              std::vector<MotiveInstance>>>
      _motiveInstances;
  std::vector<std::vector<float>> _lastExperiments;
  std::optional<std::vector<float>> _currentExperiment;
  std::vector<float> _lastExperimentsLogDurations;
  std::optional<size_t> _lastGuess;
  int _prevNoteonTick = 0;
};
} // namespace saint
