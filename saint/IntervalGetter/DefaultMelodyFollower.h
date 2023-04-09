#pragma once

#include "DefaultMelodyFollower.h"
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace saint {

class ObservationLikelihoodGetter {
public:
  virtual std::unordered_map<int, float> getObservationLogLikelihoods(
      const std::vector<std::pair<float, float>> &observationSamples) = 0;
  virtual ~ObservationLikelihoodGetter() = default;
};

class DefaultMelodyFollowerHelper {
public:
  static std::optional<int>
  getIndexOfLastSnippetElement(const std::vector<int> &melody,
                               std::vector<int> snippet,
                               const std::optional<int> &aroundIndex);

  std::vector<int> static getMelody(
      const std::vector<std::pair<float, std::optional<int>>> &input);

  static std::vector<int> getIntervals(const std::vector<int> &melody);

  static std::vector<std::set<std::vector<int>>>
  getUniqueIntervals(const std::vector<int> &intervals);
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
  const std::vector<int> _intervals;
  const std::vector<std::set<std::vector<int>>> _uniqueIntervals;
  std::vector<std::pair<float, float>> _observationSamples;
};
} // namespace saint
