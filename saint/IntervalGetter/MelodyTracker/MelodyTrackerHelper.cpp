#include "MelodyTrackerHelper.h"

#include <algorithm>
#include <cassert>
#include <map>

namespace saint {
namespace {
void updateMatched(const std::vector<int> &intervals,
                   const std::vector<int> &match, int order,
                   std::vector<bool> &matched) {
  assert(intervals.size() == matched.size());
  auto it = intervals.begin();
  while (true) {
    it = std::search(it, intervals.end(), match.begin(), match.end());
    if (it == intervals.end()) {
      break;
    }
    const auto offset = std::distance(intervals.begin(), it);
    std::fill(matched.begin() + offset, matched.begin() + offset + order + 1u,
              true);
    ++it;
  }
}
} // namespace

std::vector<int> MelodyTrackerHelper::getMelody(
    const std::vector<std::pair<float, std::optional<int>>> &input) {
  std::vector<int> melody;
  melody.reserve(input.size());
  for (const auto &entry : input) {
    if (entry.second.has_value()) {
      melody.push_back(*entry.second);
    }
  }
  return melody;
}

std::vector<std::set<std::vector<int>>>
MelodyTrackerHelper::getUniqueIntervals(const std::vector<int> &intervals) {
  std::vector<bool> matched(intervals.size());
  std::vector<std::set<std::vector<int>>> uniques;
  while (
      !std::all_of(matched.begin(), matched.end(), [](bool m) { return m; })) {
    std::map<std::vector<int>, int> map;
    const auto order = uniques.size();
    if (order >= intervals.size()) {
      break;
    }
    for (auto i = 0u; i < intervals.size() - order - 1u; ++i) {
      map[{intervals.begin() + i, intervals.begin() + i + order + 1u}]++;
    }
    std::set<std::vector<int>> newSet;
    for (const auto &entry : map) {
      if (entry.second == 1) {
        const auto &newMatch = entry.first;
        auto seenAlready = false;
        for (const auto &set : uniques) {
          for (const auto &oldMatch : set) {
            if (std::search(newMatch.begin(), newMatch.end(), oldMatch.begin(),
                            oldMatch.end()) != newMatch.end()) {
              updateMatched(intervals, newMatch, order, matched);
              seenAlready = true;
              break;
            }
          }
        }
        if (!seenAlready) {
          newSet.insert(newMatch);
        }
      }
    }
    for (const auto &match : newSet) {
      updateMatched(intervals, match, order, matched);
    }
    uniques.emplace_back(std::move(newSet));
  }
  return uniques;
}

std::vector<int>
MelodyTrackerHelper::getIntervals(const std::vector<int> &melody) {
  std::vector<int> intervals(melody.size() - 1u);
  for (auto i = 1u; i < melody.size(); ++i) {
    intervals[i - 1] = melody[i] - melody[i - 1];
  }
  return intervals;
}

std::optional<int> MelodyTrackerHelper::getIndexOfLastSnippetElement(
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
} // namespace saint