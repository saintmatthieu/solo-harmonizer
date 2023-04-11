#pragma once

#include "TimingEstimator.h"

#include <chrono>
#include <vector>

namespace saint {
class DefaultTimingEstimator : public TimingEstimator {
public:
  DefaultTimingEstimator(std::vector<float> onsetTimes);
  void addAttack(const std::chrono::milliseconds &, size_t noteIndex) override;
  float estimateNoteIndex(const std::chrono::milliseconds &) override;

private:
  static constexpr auto fittingOrder = 4u;
  const std::vector<float> _onsetTimes;
  std::vector<std::pair<std::chrono::milliseconds, float>> _coordinates;
};
} // namespace saint