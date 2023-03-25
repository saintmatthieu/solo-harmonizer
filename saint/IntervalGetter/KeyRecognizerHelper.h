#pragma once

#include "CommonTypes.h"

#include <map>

namespace saint {
class KeyRecognizerHelper {
public:
  static std::map<size_t /*bar index*/, size_t /*end crotchet index*/>
  groupIndicesByBar(const std::vector<float> &crotchets,
                    const std::vector<SigPos> &sigs);
};
} // namespace saint
