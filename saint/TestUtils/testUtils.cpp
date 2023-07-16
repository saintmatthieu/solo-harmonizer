#include "testUtils.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <filesystem>
#include <memory>
#include <optional>

namespace saint {
namespace testUtils {

namespace fs = std::filesystem;

std::unique_ptr<juce::AudioFormatWriter>
getJuceWavFileWriter(const fs::path &path, int sampleRate) {
  juce::WavAudioFormat format;
  std::unique_ptr<juce::AudioFormatWriter> writer;
  if (fs::exists(path)) {
    fs::remove(path);
  }
  writer.reset(format.createWriterFor(
      new juce::FileOutputStream(juce::File(path.string())),
      static_cast<double>(sampleRate), 1, 16, {}, 0));
  return writer;
}

void toWavFile(const float *audio, size_t N, std::optional<fs::path> pathOpt,
               int sampleRate) {
  const auto writer = getJuceWavFileWriter(
      pathOpt ? *pathOpt : fs::path{"C:/Users/saint/Downloads/test.wav"},
      sampleRate);
  writer->writeFromFloatArrays(&audio, 1, (int)N);
}

std::string getInputFilePath() {
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

std::string getOutDir() { return "C:/Users/saint/Downloads/"; }

std::unique_ptr<juce::AudioFormatReader>
getJuceWavFileReader(const fs::path &path) {
  juce::WavAudioFormat format;
  std::unique_ptr<juce::AudioFormatReader> reader;
  reader.reset(format.createReaderFor(
      new juce::FileInputStream(juce::File(path.string())), true));
  return reader;
}

std::vector<float> fromWavFile(std::optional<fs::path> pathOpt,
                               int &sampleRate) {
  const auto reader =
      getJuceWavFileReader(pathOpt ? *pathOpt : fs::path{getInputFilePath()});
  std::vector<float> audio((size_t)reader->lengthInSamples);
  const auto pData = audio.data();
  reader->read(&pData, 1, 0, (int)reader->lengthInSamples);
  sampleRate = reader->sampleRate;
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
} // namespace saint
