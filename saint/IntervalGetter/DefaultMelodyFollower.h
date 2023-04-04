#pragma once

#include "DefaultMelodyFollower.h"
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace saint {
class ObservationLikelihoodGetter {
public:
  virtual std::unordered_map<int, float> getObservationLikelihoods(
      const std::vector<std::pair<float, float>> &observationSamples) = 0;
  virtual ~ObservationLikelihoodGetter() = default;
};

class DefaultMelodyFollowerHelper {
public:
  static std::optional<int>
  getIndexOfLastSnippetElement(const std::vector<int> &melody,
                               std::vector<int> snippet,
                               const std::optional<int> &aroundIndex);
};

class DefaultMelodyFollower {
public:
  DefaultMelodyFollower(
      ObservationLikelihoodGetter &,
      const std::vector<std::pair<float, std::optional<int>>> &melody);

  void addNoteSample(float time, float noteNumber);
  std::optional<int> getNextNoteIndex();

private:
  ObservationLikelihoodGetter &_likelihoodGetter;
  const std::vector<int> _melody;
  const std::unordered_set<int> _melodyNoteNumberSet;
  const std::unordered_map<int, float> _initialPriors;
  const std::unordered_map<int, std::unordered_map<int, float>>
      _transitionLikelihoods;
  std::unordered_map<int, float> _priors;
  std::vector<std::unordered_map<int, int>> _paths;
  bool _first = true;
  std::optional<int> _lastReturnedIndex;
  std::vector<std::pair<float, float>> _observationSamples;
};
} // namespace saint
