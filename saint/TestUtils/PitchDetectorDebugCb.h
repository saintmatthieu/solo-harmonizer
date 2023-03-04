#pragma once

#include <functional>
#include <optional>

namespace saint {
namespace testUtils {
struct PitchDetectorFftAnal {
  int windowSize;
  std::vector<float> xcor;
  int olapAnalIndex;
  int peakIndex;
  float scaledMax;
  float maxMin;
};

struct PitchDetectorDebugCbArgs {
  std::vector<PitchDetectorFftAnal> anal;
  std::optional<float> detectedPitch;
  int blockSize;
};

using PitchDetectorDebugCb =
    std::function<void(const PitchDetectorDebugCbArgs &)>;

PitchDetectorDebugCb getPitchDetectorDebugCb();
} // namespace testUtils
} // namespace saint