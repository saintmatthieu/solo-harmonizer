#pragma once

#include "MelodyTracker/MelodyRecognizer/MelodyRecognizer.h"
#include "ObservationLikelihoodGetter/ObservationLikelihoodGetter.h"
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace saint {
class DefaultMelodyRecognizer : public MelodyRecognizer {
public:
  DefaultMelodyRecognizer(
      std::unique_ptr<ObservationLikelihoodGetter>,
      const std::vector<std::pair<float, std::optional<int>>> &melody);
  bool onNoteOff(const std::vector<std::pair<std::chrono::milliseconds, float>>
                     &) override;
  size_t getNextNoteIndex() override;

private:
  const std::unique_ptr<ObservationLikelihoodGetter> _likelihoodGetter;
  const std::vector<int> _melody;
  const std::vector<int> _intervals;
  // Outmost vector has one entry per sequence order, i.e., index 0 => sequences
  // of length 1, index 1 => sequences of length 2, etc.
  // The map maps the begin index of the sub-vector in _intervals to the
  // sub-vector itself.

  const std::vector<std::map<size_t,            // _intervals sub-vector index
                             std::vector<int>>> // _intervals sub-vector
      _uniqueSequences;
  // const std::vector<std::map<size_t, std::vector<std::optional<float>>>>
  //     _intervalLikelihoods;
  const std::map<int, std::set<int>> _intervalTransitions;
  std::map<int, float> _likelihoodPathTips;
  std::optional<size_t> _nextNoteIndex;
  std::vector<std::map<int, float>> _pastLikelihoods;
};
} // namespace saint
