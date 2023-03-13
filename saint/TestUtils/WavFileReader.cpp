#include "WavFileReader.h"

#include "testUtils.h"

namespace saint {
namespace testUtils {
namespace fs = std::filesystem;

WavFileReader::WavFileReader(const fs::path &path)
    : _juceReader(getJuceWavFileReader(path)) {}

int WavFileReader::getNumSamplesAvailable() const {
  return std::max(
      static_cast<int>(_juceReader->lengthInSamples) - _numReadSamples, 0);
}

int WavFileReader::getSampleRate() const {
  return static_cast<int>(_juceReader->sampleRate);
}

void WavFileReader::read(float *audio, int size) {
  std::vector<float *> channels(1);
  channels[0] = audio;
  _juceReader->read(channels.data(), 1,
                    static_cast<juce::int64>(_numReadSamples), size);
  _numReadSamples += size;
}
} // namespace testUtils
} // namespace saint
