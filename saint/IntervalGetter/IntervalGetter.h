#pragma once

#include "CommonTypes.h"

#include <memory>
#include <optional>
#include <vector>

namespace saint {
class IntervalGetter {
public:
  virtual ~IntervalGetter() = default;

  static std::shared_ptr<IntervalGetter>
  createInstance(const std::vector<IntervalSpan> &,
                 const std::optional<int> &samplesPerSecond,
                 const std::optional<float> &crotchetsPerSecond);

  // The caller should only pass a nullopt pitch whenever there is no pitch to
  // be had, or a pitch jump was detected.
  // When `pitch == nullopt`, the algorithm will allow pitch shift jumps.
  virtual std::optional<float>
  getHarmoInterval(float timeInCrotchets, const std::optional<float> &pitch,
                   int blockSize = 0) = 0;
};
} // namespace saint
