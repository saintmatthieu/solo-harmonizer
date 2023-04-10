#pragma once

#include "TimingEstimator.h"
#include <chrono>
#include <ratio>

namespace saint {
class DefaultTimingEstimator : public TimingEstimator {
public:
  void addAttack(const std::chrono::milliseconds &, size_t noteIndex) override;
  float estimateNoteIndex(const std::chrono::milliseconds &) override;
};
} // namespace saint