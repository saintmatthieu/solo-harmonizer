#pragma once

#include <map>
#include <optional>
#include <set>
#include <vector>

namespace saint {
class MelodyTrackerHelper {
public:
  static std::optional<int>
  getIndexOfLastSnippetElement(const std::vector<int> &melody,
                               std::vector<int> snippet,
                               const std::optional<int> &aroundIndex);

  std::vector<int> static getMelody(
      const std::vector<std::pair<float, std::optional<int>>> &input);

  static std::vector<int> getIntervals(const std::vector<int> &melody);

  static std::map<int, std::set<int>>
  getTransitions(const std::vector<int> &sequence);
};

} // namespace saint