#include "DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyTrackerHelper.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <set>

namespace saint {
std::unique_ptr<MelodyRecognizer> MelodyRecognizer::createInstance(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  return std::make_unique<DefaultMelodyRecognizer>(
      ObservationLikelihoodGetter::createInstance(melody), melody);
}

namespace {
auto getInitializedIntervalLikelihoods(
    const std::vector<std::map<size_t, std::vector<int>>> &uniqueSequences) {
  std::vector<std::map<size_t, std::vector<std::optional<float>>>>
      intervalLikelihoods(uniqueSequences.size());
  for (auto order = 0u; order < uniqueSequences.size(); ++order) {
    const auto &sequenceEntries = uniqueSequences[order];
    auto &sequenceLikelihoods = intervalLikelihoods[order];
    for (const auto &entry : sequenceEntries) {
      const auto sequenceIndex = entry.first;
      const auto &intervals = entry.second;
      sequenceLikelihoods[sequenceIndex].resize(order + 1);
    }
  }
  return intervalLikelihoods;
}

size_t getUniqueSequenceCount(
    const std::vector<std::map<size_t, std::vector<int>>> &uniqueSequences) {
  size_t count = 0u;
  for (const auto &map : uniqueSequences) {
    count += map.size();
  }
  return count;
}
} // namespace

DefaultMelodyRecognizer::DefaultMelodyRecognizer(
    std::unique_ptr<ObservationLikelihoodGetter> likelihoodGetter,
    const std::vector<std::pair<float, std::optional<int>>> &melody)
    : _likelihoodGetter(std::move(likelihoodGetter)),
      _melody(MelodyTrackerHelper::getMelody(melody)),
      _intervals(MelodyTrackerHelper::getIntervals(_melody)),
      _uniqueSequences(MelodyTrackerHelper::getUniqueIntervals(_intervals)),
      _uniqueSequenceCount(getUniqueSequenceCount(_uniqueSequences)),
      // _intervalLikelihoods(getInitializedIntervalLikelihoods(_uniqueSequences)),
      _intervalTransitions(MelodyTrackerHelper::getTransitions(_intervals)) {}

bool DefaultMelodyRecognizer::onNoteOff(
    const std::vector<std::pair<std::chrono::milliseconds, float>>
        &noteonSamples) {
  if (_pastLikelihoods.size() == _uniqueSequences.size()) {
    _pastLikelihoods.erase(_pastLikelihoods.begin());
  }
  _pastLikelihoods.emplace_back(
      _likelihoodGetter->getObservationLogLikelihoods(noteonSamples));
  const auto maxOrder =
      std::min(_pastLikelihoods.size(), _uniqueSequences.size());
  struct Score {
    size_t order;
    size_t orderSequenceIndex;
    float totalLikelihood;
  };
  std::vector<Score> scores;
  scores.reserve(_uniqueSequenceCount);
  for (size_t order = 1u; order <= maxOrder; ++order) {
    const auto &sequenceEntries = _uniqueSequences[order - 1u];
    for (const auto &entry : sequenceEntries) {
      const auto sequenceIndex = entry.first;
      const auto &sequence = entry.second;
      assert(sequence.size() == order);
      auto totalLikelihood = 0.f;
      for (auto i = 0u; i < sequence.size(); ++i) {
        const auto interval = sequence[i];
        const auto &likelihoods = _pastLikelihoods[i];
        totalLikelihood += likelihoods.at(interval);
      }
      scores.emplace_back(order, sequenceIndex, totalLikelihood);
    }
  }
  const auto &winner = *std::max_element(
      scores.begin(), scores.end(), [](const Score &lhs, const Score &rhs) {
        return lhs.totalLikelihood < rhs.totalLikelihood;
      });
  const auto noteIndex = winner.orderSequenceIndex;
  return false;
}

size_t DefaultMelodyRecognizer::getNextNoteIndex() {
  assert(_nextNoteIndex.has_value());
  return _nextNoteIndex.has_value() ? *_nextNoteIndex : 0u;
}
} // namespace saint