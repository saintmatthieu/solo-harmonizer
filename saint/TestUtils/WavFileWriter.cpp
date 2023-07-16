#include "WavFileWriter.h"

#include "testUtils.h"

namespace saint {
namespace testUtils {
namespace fs = std::filesystem;

WavFileWriter::WavFileWriter(const fs::path &path, int sampleRate)
    : _juceWriter(getJuceWavFileWriter(path, sampleRate)) {}

bool WavFileWriter::write(const float *audio, int size) {
  std::vector<const float *> channels(1);
  channels[0] = audio;
  return _juceWriter->writeFromFloatArrays(channels.data(), 1, size);
}

bool WavFileWriter::write(const std::vector<float> &audio) {
  return write(audio.data(), static_cast<int>(audio.size()));
}

bool WavFileWriter::write(float value, int size) {
  std::vector<float> audio(size);
  std::fill(audio.begin(), audio.end(), value);
  return write(audio);
}
} // namespace testUtils
} // namespace saint
