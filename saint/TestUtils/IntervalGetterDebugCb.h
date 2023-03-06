#pragma once

#include <functional>
#include <optional>

namespace saint {
namespace testUtils {
struct IntervalGetterDebugCbArgs {
  const std::vector<int> &intervalTicks;
  const std::optional<float> &inputPitch;
  const std::optional<float> &returnedInterval;
  float ticksPerSample;
  int newIndex;
  int blockSize;
};

using IntervalGetterDebugCb =
    std::function<void(const IntervalGetterDebugCbArgs &)>;

IntervalGetterDebugCb getIntervalGetterDebugCb();
} // namespace testUtils
} // namespace saint