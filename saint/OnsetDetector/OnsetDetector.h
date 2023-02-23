#pragma once

#include <pffft.hpp>
#include <ringbuffer.hpp>

#include <array>

namespace saint {
class OnsetDetector {
public:
  // Don't even try instantiating me if the block size exceeds this.
  static constexpr auto maxBlockSize = 8192;
  OnsetDetector(int sampleRate);
  bool process(const float *, int);

private:
  // PFFT memory alignment requirement
  template <typename T> struct alignas(16) Aligned {
    T value;
  };
  // (zero-padded) FFT input
  Aligned<std::vector<float>> _timeData;
  // FFT output
  Aligned<std::vector<std::complex<float>>> _freqData;
  Aligned<std::vector<std::complex<float>>> _autoCorrFreqData;

  // public for testing
public:
  // time-domain cross-correlation of current and past block
  Aligned<std::vector<float>> _autoCorrTimeData;
  float _peakMax = 0.f;
  int _peakMaxIndex = 0;

private:
  const std::vector<float> _window;
  const int _fftSizeSamples;
  const int _firstSearchIndex;
  const int _lastSearchIndex;
  pffft::Fft<float> _fftEngine;
  jnk0le::Ringbuffer<float, maxBlockSize> _ringBuffer;
  const std::vector<float> _windowXCorr;
};
} // namespace saint
