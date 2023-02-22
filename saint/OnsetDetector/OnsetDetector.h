#pragma once

#include "OnsetsDS/onsetsds.h"

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
  struct alignas(16) AlignedArray { // To meet PFFFT's alignment requirements
    std::array<float, maxBlockSize> array;
  };
  const std::vector<float> _window;
  const int _fftSizeSamples;
  OnsetsDS _onsetsDS;
  std::vector<float> _onsetsDSBuffer;
  pffft::Fft<float> _fftEngine;
  std::vector<pffft::Fft<float>::Complex> _fftData;
  int _ringBufferIndex = 0;
  std::array<jnk0le::Ringbuffer<float, maxBlockSize>, 2> _ringBuffers;
  int _numSamplesRemoved = 0;
  AlignedArray _readBuffer;
};
} // namespace saint
