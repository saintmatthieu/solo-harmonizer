#include "RingBuffer.h"

namespace DavidCNAntonia {
RingBuffer::RingBuffer() {}

void RingBuffer::initialise(int numChannels, int numSamples) {
  readPos.resize(numChannels);
  writePos.resize(numChannels);

  for (int i = 0; i < readPos.size(); i++) {
    readPos[i] = 0.0;
    writePos[i] = 0.0;
  }

  buffer.setSize(numChannels, numSamples);
  pointerArrayBuffer.setSize(numChannels, numSamples);
}

void RingBuffer::pushSample(float sample, int channel) {
  buffer.setSample(channel, writePos[channel], sample);

  if (++writePos[channel] >= buffer.getNumSamples()) {
    writePos[channel] = 0;
  }
}

float RingBuffer::popSample(int channel) {
  auto sample = buffer.getSample(channel, readPos[channel]);

  if (++readPos[channel] >= buffer.getNumSamples()) {
    readPos[channel] = 0;
  }
  return sample;
}

int RingBuffer::getAvailableSamples(int channel) {
  if (readPos[channel] <= writePos[channel]) {
    return writePos[channel] - readPos[channel];
  } else {
    return writePos[channel] + buffer.getNumSamples() - readPos[channel];
  }
}
const float *const *RingBuffer::readPointerArray(int reqSamples) {
  for (int sample = 0; sample < reqSamples; sample++) {
    for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
      pointerArrayBuffer.setSample(channel, sample, popSample(channel));
    }
  }
  return pointerArrayBuffer.getArrayOfReadPointers();
}

float *const *RingBuffer::writePointerArray() {
  return pointerArrayBuffer.getArrayOfWritePointers();
}

void RingBuffer::copyToBuffer(int numSamples) {
  for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
    for (int sample = 0; sample < numSamples; sample++) {
      pushSample(pointerArrayBuffer.getSample(channel, sample), channel);
    }
  }
}
} // namespace DavidCNAntonia