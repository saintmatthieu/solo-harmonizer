#pragma once

#include <functional>
#include <optional>

namespace saint {
namespace testUtils {
using PerformanceTimeWarperDebugCb = std::function<void(
    float timeInCrotchets, const std::optional<float> &pitch, float retValue)>;

void setPerformanceTimeWarperDebugCbParams(int samplesPerBlock,
                                           float crotchetsPerSecond);
void resetPerformanceTimeWarperDebugCb();
PerformanceTimeWarperDebugCb *getPerformanceTimeWarperDebugCb();
} // namespace testUtils
} // namespace saint