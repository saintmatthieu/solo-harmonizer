#pragma once

#include <pffft.hpp>
#include <ringbuffer.hpp>

#include <array>
#include <functional>
#include <optional>

namespace saint {

using OnXcorReady =
    std::function<void(const std::vector<float> &xcorr, int windowSize,
                       int ringBufferIndex, int peakIndex, float scaledMax)>;

// PFFT memory alignment requirement
template <typename T> struct alignas(16) Aligned {
  T value;
};

class OnsetDetector {
public:
  // Don't even try instantiating me if the block size exceeds this.
  // An electric guitar's lower E is normally 83Hz.
  // For a spectrum using a 1st-order cosine window, each sinusoid lobe spans 2
  // bins. That requires a window of ~2/83s=25ms.
  static constexpr auto windowSizeMs = 25;
  static constexpr auto maxBlockSize = 8192;
  OnsetDetector(int sampleRate, std::optional<OnXcorReady> = std::nullopt);
  bool process(const float *, int);

private:
  // (zero-padded) FFT input
  Aligned<std::vector<float>> _timeData;
  Aligned<std::vector<std::complex<float>>> _freqData;
  const std::optional<OnXcorReady> _onXcorReady;

  const std::vector<float> _window;
  const int _fftSizeSamples;
  const int _firstSearchIndex;
  const int _lastSearchIndex;
  pffft::Fft<float> _fftEngine;
  std::array<jnk0le::Ringbuffer<float, maxBlockSize>, 2> _ringBuffers;
  int _ringBufferIndex = 0;
  const std::vector<float> _windowXCorr;
};
} // namespace saint
