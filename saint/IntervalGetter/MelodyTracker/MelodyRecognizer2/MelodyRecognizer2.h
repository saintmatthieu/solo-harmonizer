#pragma once

#include <map>
#include <optional>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace saint {
class MelodyRecognizer2 {
public:
  using Melody =
      std::vector<std::pair<float /*duration*/, int /*note number*/>>;
  MelodyRecognizer2(Melody melody);
  std::optional<size_t> beginNewNote(int tickCounter);
  void addPitchMeasurement(float pc);

public: // but not meant for external usage
  struct MotiveInstance {
    int firstNoteNumber;
    float firstDuration;
  };

  struct MotiveInfo {
    float instantaneousProb = 1.f; // i.e., without considering transition probs
    std::vector<MotiveInstance> instances;
  };

  using MotiveInvariants = std::vector<
      std::pair<Melody /*motive*/,
                std::unordered_map<size_t /*begin index*/, MotiveInstance>>>;

private:
  struct Stats {
    float combinedLikelihood;
    std::vector<std::pair<float /*pitch*/, float /*duration*/>> transpositions;
  };

  const float _referenceDuration;
  const MotiveInvariants _motives;
  std::vector<std::vector<float>> _lastExperiments;
  std::optional<std::vector<float>> _currentExperiment;
  std::vector<float> _lastExperimentsLogDurations;
  std::optional<size_t> _lastGuess;
  int _prevNoteonTick = 0;
  std::vector<Stats> _prevStats;
};
} // namespace saint
