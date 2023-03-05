#pragma once

#include <functional>
#include <optional>

namespace saint {
namespace testUtils {
struct IntervalGetterDebugCbArgs {
  std::optional<float> inputPitch;
  int newIndex;
  int numIndices;
  int blockSize;
};

using IntervalGetterDebugCb =
    std::function<void(const IntervalGetterDebugCbArgs &)>;

IntervalGetterDebugCb getIntervalGetterDebugCb();
} // namespace testUtils
} // namespace saint