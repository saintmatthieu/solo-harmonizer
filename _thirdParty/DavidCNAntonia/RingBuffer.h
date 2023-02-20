#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace DavidCNAntonia {
class RingBuffer {
public:
  RingBuffer();
  void initialise(int numChannels, int numSamples);
  void pushSample(float sample, int channel);
  float popSample(int channel);
  int getAvailableSamples(int channel);
  const float *const *readPointerArray(int reqSamples);
  float *const *writePointerArray();
  void copyToBuffer(int numSamples);

private:
  juce::AudioBuffer<float> buffer, pointerArrayBuffer;
  std::vector<int> readPos, writePos;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingBuffer)
};
} // namespace DavidCNAntonia