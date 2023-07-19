#pragma once

#include "CommonTypes.h"

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace saint {
class IntervalGetter {
public:
  // The caller should only pass a nullopt pitch whenever there is no pitch to
  // be had, or a pitch jump was detected.
  // When `pitch == nullopt`, the algorithm will allow pitch shift jumps.
  virtual std::optional<float>
  getHarmoInterval(float timeInCrotchets,
                   const std::optional<std::function<float(int)>> &getPitchLlh,
                   const std::chrono::milliseconds &now,
                   std::optional<size_t> &melodyRecognizerDebugOut,
                   int blockSize = 0) = 0;

  virtual ~IntervalGetter() = default;
};
} // namespace saint
