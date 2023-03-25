#pragma once

#include "CommonTypes.h"

namespace saint {
class KrumhanslSchmucklerAlgorithm {
public:
  static std::vector<std::pair<Key, float>>
  getLikelihoods(const std::vector<int> &noteNumbers);
  static Key getMostLikelyKey(const std::vector<int> &noteNumbers);
};
} // namespace saint
