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
  struct Stats {
    float prob;
    float pitchTranspose;
    float durationTranspose;
  };

  struct MotiveInstance {
    int firstNoteNumber;
    float firstDuration;
    mutable Stats currStats;
    mutable std::optional<Stats> prevStats;
  };

  using MotiveInvariant =
      std::pair<Melody /*motive*/,
                std::unordered_map<size_t /*begin index*/, MotiveInstance>>;
  using MotiveInvariants = std::vector<MotiveInvariant>;

  struct TableRow {
    const Melody motive;
    const size_t beginIndex;
    const std::shared_ptr<int> firstNoteNumber;
    const std::shared_ptr<int> firstDuration;
    std::shared_ptr<Stats> currStats;
    std::shared_ptr<Stats> prevStats;
  };

private:
  const float _referenceDuration;
  const MotiveInvariants _motives;
  std::vector<std::vector<float>> _lastExperiments;
  std::optional<std::vector<float>> _currentExperiment;
  std::vector<float> _lastExperimentsLogDurations;
  std::optional<size_t> _lastGuess;
  int _prevNoteonTick = 0;
};
} // namespace saint
