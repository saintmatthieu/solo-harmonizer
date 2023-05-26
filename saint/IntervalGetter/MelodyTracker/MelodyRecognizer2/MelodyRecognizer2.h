#pragma once

#include <map>
#include <memory>
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

  struct TableRow {
    TableRow(std::shared_ptr<Melody> motive, int firstNoteNumber,
             float firstDuration);
    std::shared_ptr<Melody> motive;
    int firstNoteNumber;
    float firstDuration;
    mutable std::shared_ptr<float> pitchClassErrorAvg;
    mutable std::shared_ptr<float> durationErrorAvg;
    mutable std::shared_ptr<float> prevProb;
    mutable std::shared_ptr<float> currProb;
    mutable std::optional<std::pair<float /*pitch*/, float /*duration*/>>
        prevTranspositions;
    mutable std::optional<std::pair<float /*pitch*/, float /*duration*/>>
        currTranspositions;
  };

  using MotiveInvariant =
      std::pair<Melody /*motive*/,
                std::unordered_map<size_t /*begin index*/, MotiveInstance>>;
  using MotiveInvariants = std::vector<MotiveInvariant>;

private:
  void updateTablePrevFields();

  const float _referenceDuration;
  const std::vector<TableRow> _table;
  const MotiveInvariants _motives;
  std::vector<std::shared_ptr<std::vector<float>>> _lastExperiments;
  std::shared_ptr<std::vector<float>> _currentExperiment;
  std::vector<float> _lastExperimentsLogDurations;
  std::optional<size_t> _lastGuess;
  int _prevNoteonTick = 0;
};
} // namespace saint
