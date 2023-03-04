#pragma once

#include "testUtils.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <memory>

namespace saint {
namespace testUtils {
struct PitchDetectorImplCbArgs {
  int windowSize;
  std::vector<float> xcor;
  int olapAnalIndex;
  int peakIndex;
  float scaledMax;
  float maxMin;
};

using PitchDetectorImplCb =
    std::function<void(const PitchDetectorImplCbArgs &)>;

PitchDetectorImplCb getPitchDetectorImplDebugCb();
} // namespace testUtils
} // namespace saint