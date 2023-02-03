#include "testUtils.h"

#include <juce_audio_utils/juce_audio_utils.h>

#include <filesystem>
#include <memory>
#include <optional>

namespace testUtils {
void toWavFile(const float *audio, size_t N,
               std::optional<std::filesystem::path> pathOpt) {
  juce::WavAudioFormat format;
  std::unique_ptr<juce::AudioFormatWriter> writer;
  const auto path =
      pathOpt ? pathOpt->string() : "C:/Users/saint/Downloads/test.wav";
  if (std::filesystem::exists(path)) {
    std::filesystem::remove(path);
  }
  writer.reset(format.createWriterFor(
      new juce::FileOutputStream(juce::File(path)), 44100.0, 1, 16, {}, 0));
  writer->writeFromFloatArrays(&audio, 1, (int)N);
}

std::string getInputFilePath() {
  namespace fs = std::filesystem;
  const fs::path wavFileDir{"C:/Users/saint/Downloads/"};
  const auto defaultPath = fs::path{wavFileDir}.append("cosine.wav");
#ifdef _WIN32
  const auto path =
      __argc < 2 ? defaultPath : fs::path{wavFileDir}.append(__argv[1]);
#else
  const auto path = defaultPath;
#endif
  return path.string();
}

std::vector<float> fromWavFile() {
  juce::WavAudioFormat format;
  std::unique_ptr<juce::AudioFormatReader> reader;
  const auto path = getInputFilePath();
  reader.reset(format.createReaderFor(
      new juce::FileInputStream(juce::File(path)), true));
  std::vector<float> audio((size_t)reader->lengthInSamples);
  const auto pData = audio.data();
  reader->read(&pData, 1, 0, (int)reader->lengthInSamples);
  return audio;
}

std::vector<float> makeCosine(size_t T, size_t N) {
  std::vector<float> sinewave(N);
  const auto dT = (double)T;
  const auto dN = (double)N;
  for (auto i = 0u; i < N; ++i) {
    sinewave[i] = std::cosf((float)(i * 2 * 3.1416 * dT / dN));
  }
  return sinewave;
}

float getRms(const std::vector<float> &V) {
  auto U = V;
  std::transform(V.begin(), V.end(), U.begin(), [](auto v) { return v * v; });
  return std::sqrtf(std::accumulate(U.begin(), U.end(), 0.f) / (float)V.size());
}
} // namespace testUtils
