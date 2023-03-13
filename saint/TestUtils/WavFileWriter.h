#pragma once

#include <filesystem>
#include <memory>

#include <juce_audio_utils/juce_audio_utils.h>

namespace saint {
namespace testUtils {
class WavFileWriter {
public:
  WavFileWriter(const std::filesystem::path &path);
  bool write(const float *, int);
  bool write(const std::vector<float> &);
  bool write(float value, int xTimes);

private:
  const std::unique_ptr<juce::AudioFormatWriter> _juceWriter;
};
} // namespace testUtils
} // namespace saint