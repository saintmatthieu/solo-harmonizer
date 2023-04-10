#include "DefaultTimingEstimator.h"

namespace saint {
void DefaultTimingEstimator::addAttack(const std::chrono::milliseconds &time,
                                       size_t noteIndex) {}

float DefaultTimingEstimator::estimateNoteIndex(
    const std::chrono::milliseconds &) {
  return 0.f;
}
} // namespace saint