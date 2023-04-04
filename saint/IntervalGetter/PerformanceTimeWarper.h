#pragma once

#include <memory>
#include <optional>
#include <vector>

namespace saint {
class PerformanceTimeWarper {
public:
  static std::unique_ptr<PerformanceTimeWarper>
  createInstance(const std::vector<std::pair<float, std::optional<int>>>
                     &timedNoteNumbers);

  virtual float getWarpedTime(float timeInCrotchets,
                              const std::optional<float> &noteNumber) = 0;

  virtual ~PerformanceTimeWarper() = default;
};
} // namespace saint
