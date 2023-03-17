#pragma once

#include <functional>
#include <optional>

namespace saint {
namespace testUtils {
struct IntervalGetterDebugCbArgs {
  const std::vector<int> &intervalTicks;
  const std::optional<float> &inputPitch;
  const std::optional<float> &returnedInterval;
  int newIndex;
  int blockSize;
};

using IntervalGetterDebugCb =
    std::function<void(const IntervalGetterDebugCbArgs &)>;

IntervalGetterDebugCb getIntervalGetterDebugCb(int ticksPerSample);
} // namespace testUtils
} // namespace saint