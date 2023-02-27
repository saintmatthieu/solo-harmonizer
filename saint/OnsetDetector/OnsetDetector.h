#pragma once

#include <pffft.hpp>
#include <ringbuffer.hpp>

#include <array>
#include <functional>
#include <optional>

namespace saint {

struct OnXcorReadyArgs {
  int windowSize;
  std::vector<float> xcor;
  int olapAnalIndex;
  int peakIndex;
  float scaledMax;
  float maxMin;
};

using OnXcorReady = std::function<void(const OnXcorReadyArgs &)>;

// PFFT memory alignment requirement
template <typename T> struct alignas(16) Aligned {
  T value;
};

class OnsetDetector {
public:
  // Don't even try instantiating me if the block size exceeds this.
  static constexpr auto maxBlockSize = 8192;
  OnsetDetector(int sampleRate, std::optional<OnXcorReady> = std::nullopt);
  bool process(const float *, int);

private:
  const std::optional<OnXcorReady> _onXcorReady;
  const std::vector<float> _window;
  const int _fftSize;
  pffft::Fft<float> _fwdFft;
  std::array<jnk0le::Ringbuffer<float, maxBlockSize>, 2> _ringBuffers;
  std::array<float, 2> _maxima;
  int _ringBufferIndex = 0;
  const std::vector<float> _lpWindow;
  const std::vector<float> _windowXcor;
  const int _lastSearchIndex;
};
} // namespace saint
