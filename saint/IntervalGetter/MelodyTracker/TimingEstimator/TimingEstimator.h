#pragma once

#include <chrono>

namespace saint {
class TimingEstimator {
public:
  virtual void addAttack(const std::chrono::milliseconds &,
                         size_t noteIndex) = 0;
  virtual float estimateNoteIndex(const std::chrono::milliseconds &) = 0;
  virtual ~TimingEstimator() = default;
};
} // namespace saint