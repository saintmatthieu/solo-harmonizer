#pragma once

#include "CommonTypes.h"
#include <unordered_set>

namespace saint {
class KrumhanslSchmucklerAlgorithm {
public:
  static std::vector<std::pair<Key, float>>
  getKeyLikelihoods(const std::unordered_set<int> &noteNumbers);
  static Key getMostLikelyKey(const std::unordered_set<int> &noteNumbers);
};
} // namespace saint
