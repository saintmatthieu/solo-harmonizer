#pragma once

#include "PitchDetector.h"
#include "PitchDetectorDebugCb.h"

#include <pffft.hpp>
#include <ringbuffer.hpp>

#include <array>
#include <functional>
#include <optional>

namespace saint {

// PFFT memory alignment requirement
template <typename T> struct alignas(16) Aligned { T value; };

class PitchDetectorImpl : public PitchDetector {
public:
  // Don't even try instantiating me if the block size exceeds this.
  PitchDetectorImpl(int sampleRate,
                    const std::optional<float> &leastFrequencyToDetect,
                    std::optional<testUtils::PitchDetectorDebugCb>);
  std::optional<float> process(const float *, int, float *pitchConfidence) override;

private:
  const float _sampleRate;
  const std::optional<testUtils::PitchDetectorDebugCb> _debugCb;
  const std::vector<float> _window;
  const int _fftSize;
  pffft::Fft<float> _fwdFft;
  std::array<jnk0le::Ringbuffer<float, maxBlockSize>, 2> _ringBuffers;
  std::array<float, 2> _maxima;
  int _ringBufferIndex = 0;
  const std::vector<float> _lpWindow;
  const std::vector<float> _windowXcor;
  const int _lastSearchIndex;
  std::optional<float> _detectedPitch;
};
} // namespace saint
