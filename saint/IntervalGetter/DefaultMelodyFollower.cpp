#include "DefaultMelodyFollower.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace saint {
std::optional<int> DefaultMelodyFollowerHelper::getIndexOfLastSnippetElement(
    const std::vector<int> &melody, std::vector<int> snippet,
    const std::optional<int> &aroundIndex) {
  // Find match closest to _lastReturnedIndex
  auto melodyIt = melody.begin();
  auto melodyIndex = 0;
  auto found = false;
  while (true) {
    const auto nextMelodyIt =
        std::search(melodyIt, melody.end(), snippet.begin(), snippet.end());
    if (nextMelodyIt == melody.end()) {
      if (found) {
        break;
      }
      // No match ? Try to find a match for a smaller sequence.
      snippet.erase(snippet.begin());
      if (snippet.empty()) {
        // Still not ?? Oh my ...
        break;
      }
      continue;
    }
    found = true;
    const auto nextIndex = std::distance(melody.begin(), nextMelodyIt);
    if (!aroundIndex.has_value()) {
      // Works - first match
      melodyIndex = nextIndex;
      break;
    }
    const auto offset = *aroundIndex - static_cast<int>(snippet.size()) + 1;
    if (std::abs(nextIndex - offset) > std::abs(melodyIndex - offset)) {
      break;
    }
    melodyIndex = nextIndex;
    melodyIt = std::next(nextMelodyIt);
  }
  if (!found) {
    return std::nullopt;
  }
  return melodyIndex + static_cast<int>(snippet.size()) - 1;
}

namespace {
std::unordered_set<int> getNoteNumberSet(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  std::unordered_set<int> nns;
  for (const auto &entry : melody) {
    if (entry.second.has_value()) {
      nns.insert(*entry.second);
    }
  }
  return nns;
}

std::unordered_map<int, float> getInitialPriors(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  std::unordered_map<int, int> noteCounts;
  int totalCount = 0;
  for (const auto &entry : melody) {
    if (!entry.second.has_value()) {
      continue;
    }
    const auto nn = *entry.second;
    if (noteCounts.count(nn) == 0) {
      noteCounts.emplace(nn, 0);
    }
    ++noteCounts.at(nn);
    ++totalCount;
  }
  std::unordered_map<int, float> priors;
  for (const auto &entry : noteCounts) {
    priors.emplace(entry.first, static_cast<float>(entry.second) /
                                    static_cast<float>(totalCount));
  }
  return priors;
}

const std::unordered_map<int, std::unordered_map<int, float>>
getTransitionLikelihoods(
    const std::vector<std::pair<float, std::optional<int>>> &melodyWithSilences,
    const std::unordered_set<int> &melodyNoteNumberSet,
    const std::unordered_map<int, float> &initialPriors) {
  std::unordered_map<int, std::unordered_map<int, float>> likelihoods;
  std::vector<float> melody;
  for (const auto &entry : melodyWithSilences) {
    if (entry.second.has_value()) {
      melody.push_back(*entry.second);
    }
  }
  for (auto i = 0u; i < melody.size() - 1u; ++i) {
    if (likelihoods.count(melody[i]) == 0) {
      likelihoods[melody[i]] = {};
    }
    auto &column = likelihoods[melody[i]];
    if (column.count(melody[i + 1]) == 0) {
      column[melody[i + 1]] = 0.f;
    }
    // Record transition
    ++column.at(melody[i + 1]);
  }
  for (auto &entry : likelihoods) {
    auto &column = entry.second;
    const auto sum = std::accumulate(
        column.begin(), column.end(), 0.f,
        [](float acc, const auto &entry) { return acc + entry.second; });
    for (const auto &priorEntry : initialPriors) {
      const auto nn = priorEntry.first;
      if (column.count(nn) == 0) {
        column[nn] = 0.f;
      }
      // We normalize the column entries, but also add a bit of prior
      // probability. Although it is not the typical way of playing a melody, we
      // still want to allow the performer to jump from one note to the other.
      column.at(nn) =
          std::logf(priorEntry.second * 0.1f + column.at(nn) * 0.9f / sum);
    }
  }
  return likelihoods;
}

std::unordered_map<int, float>
getLog(const std::unordered_map<int, float> &probs) {
  std::unordered_map<int, float> logs;
  for (const auto &entry : probs) {
    logs.emplace(entry.first, std::logf(entry.second));
  }
  return logs;
}

std::vector<int>
getMelody(const std::vector<std::pair<float, std::optional<int>>> &input) {
  std::vector<int> melody;
  melody.reserve(input.size());
  for (const auto &entry : input) {
    if (entry.second.has_value()) {
      melody.push_back(*entry.second);
    }
  }
  return melody;
}
} // namespace

DefaultMelodyFollower::DefaultMelodyFollower(
    ObservationLikelihoodGetter &likelihoodGetter,
    const std::vector<std::pair<float, std::optional<int>>> &melody)
    : _likelihoodGetter(likelihoodGetter), _melody(getMelody(melody)),
      _melodyNoteNumberSet(getNoteNumberSet(melody)),
      _initialPriors(getInitialPriors(melody)),
      _transitionLikelihoods(getTransitionLikelihoods(
          melody, _melodyNoteNumberSet, _initialPriors)),
      _priors(getLog(_initialPriors)) {}

void DefaultMelodyFollower::addNoteSample(float time, float noteNumber) {
  _observationSamples.emplace_back(time, noteNumber);
}

std::optional<int> DefaultMelodyFollower::getNextNoteIndex() {
  const auto observationLikelihoods =
      _likelihoodGetter.getObservationLikelihoods(_observationSamples);
  _observationSamples.clear();
  // Should normally be fine, but for virtuoso soli with plenty of notes this
  // O(N^2) loop might become expensive ...
  std::unordered_map<int, int> newPathEntry;
  std::unordered_map<int, float> newPriors;
  for (auto j : _melodyNoteNumberSet) {
    const auto obsLikelihood = std::logf(observationLikelihoods.count(j) > 0
                                             ? observationLikelihoods.at(j)
                                             : 0.f);
    if (_first) {
      newPriors[j] = _initialPriors.at(j) + obsLikelihood;
    } else {
      std::unordered_map<int, float> probs;
      for (auto i : _melodyNoteNumberSet) {
        const auto transLikelihood = _transitionLikelihoods.at(i).at(j);
        probs[i] = _priors.at(i) + obsLikelihood + transLikelihood;
      }
      const auto bestMatchIt = std::max_element(
          probs.begin(), probs.end(), [](const auto &lhs, const auto &rhs) {
            return lhs.second < rhs.second;
          });
      newPathEntry[j] = bestMatchIt->first;
      newPriors[j] = bestMatchIt->second;
    }
  }
  _priors = newPriors;
  if (!_first) {
    _paths.push_back(std::move(newPathEntry));
  }
  auto pathNn = std::max_element(_priors.begin(), _priors.end(),
                                 [](const auto &lhs, const auto &rhs) {
                                   return lhs.second < rhs.second;
                                 })
                    ->first;
  std::vector<int> seq;
  seq.push_back(pathNn);
  for (auto rit = _paths.rbegin(); rit != _paths.rend(); ++rit) {
    pathNn = rit->at(pathNn);
    seq.push_back(pathNn);
  }
  std::reverse(seq.begin(), seq.end());
  const auto lastSequenceMatch =
      DefaultMelodyFollowerHelper::getIndexOfLastSnippetElement(
          _melody, std::move(seq), _lastReturnedIndex);
  if (!lastSequenceMatch.has_value()) {
    // TODO
  }
  _first = false;
  _lastReturnedIndex = *lastSequenceMatch + 1;
  if (_lastReturnedIndex == _melody.size()) {
    _lastReturnedIndex.reset();
  }
  return _lastReturnedIndex;
}
} // namespace saint