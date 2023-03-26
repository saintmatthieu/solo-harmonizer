#pragma once

#include <map>
#include <memory>
#include <optional>

namespace saint {
class PerformanceTimeWarper {
public:
  static std::unique_ptr<PerformanceTimeWarper>
  createInstance(const std::map<float, std::optional<int>> &timedNoteNumbers);

  virtual float getWarpedTime(float timeInCrotchets,
                              const std::optional<float> &pitch) = 0;

  virtual ~PerformanceTimeWarper() = default;
};
} // namespace saint
