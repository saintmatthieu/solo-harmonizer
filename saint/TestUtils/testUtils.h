#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <juce_audio_utils/juce_audio_utils.h>

namespace saint {
namespace testUtils {
std::vector<float>
fromWavFile(std::optional<std::filesystem::path> path = std::nullopt);

std::unique_ptr<juce::AudioFormatReader>
getWavFileReader(std::filesystem::path pathOpt);

void toWavFile(const float *audio, size_t N,
               std::optional<std::filesystem::path> pathOpt = std::nullopt);

class WavFileWriter {
public:
  WavFileWriter(const std::filesystem::path &path);
  bool write(const float *, int);
  bool write(const std::vector<float> &);
  bool write(float value, int xTimes);

private:
  const std::unique_ptr<juce::AudioFormatWriter> _juceWriter;
};

std::string getInputFilePath();
std::string getOutDir();

std::vector<float> makeCosine(size_t T, size_t N);

float getRms(const std::vector<float> &V);

template <typename T>
std::vector<T> getDiff(const std::vector<T> &A, const std::vector<T> &B) {
  std::vector<T> diff(A.size());
  for (auto i = 0u; i < A.size(); ++i) {
    diff[i] = B[i] - A[i];
  }
  return diff;
}
} // namespace testUtils
} // namespace saint