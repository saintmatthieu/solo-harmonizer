#pragma once

#include "OnsetsDS/onsetsds.h"

#include <juce_dsp/juce_dsp.h>

namespace saint {
class OnsetDetector {
public:
  OnsetDetector(int samplesRate, int blockSize);
  bool process(const float *, int);

private:
  juce::dsp::FFT _fft;
  OnsetsDS _onsetsDS;
};
} // namespace saint
