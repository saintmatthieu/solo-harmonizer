#pragma once

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

  static std::vector<std::set<std::vector<int>>>
  getUniqueIntervals(const std::vector<int> &intervals);
};

} // namespace saint