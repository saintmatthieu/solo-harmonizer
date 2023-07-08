#include "DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyTrackerHelper.h"
#include <algorithm>
#include <array>
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

DefaultMelodyRecognizer::SequenceMap
DefaultMelodyRecognizer::_getSequenceMap(const std::vector<int> &intervals) {
  DefaultMelodyRecognizer::SequenceMap sequenceMap;
  constexpr auto length =
      DefaultMelodyRecognizer::trackingIntervalSequenceLength;
  for (auto i = 0u; i < intervals.size() - length; ++i) {
    std::array<int, length> sequence;
    std::copy(intervals.begin() + i, intervals.begin() + i + length,
              sequence.begin());
    auto matchIt = intervals.begin();
    while (true) {
      matchIt = std::search(matchIt, intervals.end(), sequence.begin(),
                            sequence.end());
      if (matchIt == intervals.end()) {
        break;
      }
      sequenceMap[sequence].push_back(
          std::distance(intervals.begin(), matchIt));
      ++matchIt;
    }
  }
  return sequenceMap;
}

namespace {
auto getInitializedIntervalLikelihoods(
    const std::vector<std::map<size_t, std::vector<int>>> &uniqueSequences) {
  std::vector<std::map<size_t, std::vector<std::optional<float>>>>
      intervalLikelihoods(uniqueSequences.size());
  for (auto order = 0u; order < uniqueSequences.size(); ++order) {
    const auto &sequenceEntries = uniqueSequences[order];
    auto &sequenceLikelihoods = intervalLikelihoods[order];
    for (const auto &[sequenceIndex, intervals] : sequenceEntries) {
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
      _sequenceMap(_getSequenceMap(_intervals)),
      // _intervalLikelihoods(getInitializedIntervalLikelihoods(_sequenceMap)),
      _intervalTransitions(MelodyTrackerHelper::getTransitions(_intervals)) {}

bool DefaultMelodyRecognizer::onNoteOff(
    const std::vector<std::pair<std::chrono::milliseconds, float>>
        &noteonSamples) {
  const auto observationLikelihoods =
      _likelihoodGetter->getObservationLogLikelihoods(noteonSamples);
  if (observationLikelihoods.empty()) {
    // Apparently no samples worth looking at
    return false;
  }
  _pastLikelihoods.emplace_back(std::move(observationLikelihoods));
  if (_pastLikelihoods.size() < trackingIntervalSequenceLength) {
    return false;
  } else if (_pastLikelihoods.size() > trackingIntervalSequenceLength) {
    _pastLikelihoods.erase(_pastLikelihoods.begin());
  }
  const auto maxOrder = std::min(_pastLikelihoods.size(), _sequenceMap.size());
  std::map<size_t /*index*/, float> scores;
  const auto nextOccurrenceIndices = _getSequenceNextOccurrenceIndices();
  for (const auto &[sequence, occurrenceIndex] : nextOccurrenceIndices) {
    auto totalLikelihood = 0.f;
    for (auto i = 0u; i < sequence.size(); ++i) {
      const auto interval = sequence[i];
      const auto &likelihoods = _pastLikelihoods[i];
      totalLikelihood += likelihoods.at(interval);
    }
    scores.emplace(occurrenceIndex, totalLikelihood);
  }
  const auto &[winnerIndex, winnerScore] = *std::max_element(
      scores.begin(), scores.end(),
      [](const auto &lhs, const auto &rhs) { return lhs.second < rhs.second; });
  if (winnerScore < -5.f) {
    return false;
  }
  _winningSequenceIndex = winnerIndex;
  return true;
}

size_t DefaultMelodyRecognizer::getNextNoteIndex() {
  return _winningSequenceIndex + trackingIntervalSequenceLength + 1u;
}

DefaultMelodyRecognizer::SequenceOccurrenceIndexMap
DefaultMelodyRecognizer::_getSequenceNextOccurrenceIndices() const {
  SequenceOccurrenceIndexMap fwd;
  for (const auto &entry : _sequenceMap) {
    auto &sequenceIndices = entry.second;
    const auto candidateIt = std::lower_bound(
        sequenceIndices.begin(), sequenceIndices.end(), _winningSequenceIndex);
    if (candidateIt != sequenceIndices.end()) {
      fwd[entry.first] = *candidateIt;
    }
  }
  return fwd;
}
} // namespace saint