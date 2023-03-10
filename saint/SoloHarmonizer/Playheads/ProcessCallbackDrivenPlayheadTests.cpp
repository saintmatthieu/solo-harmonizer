#include "ProcessCallbackDrivenPlayhead.h"
#include "testUtils.h"

#include <gtest/gtest.h>

namespace saint {

constexpr auto samplesPerSecond = 44100;
constexpr auto crotchetsPerSecond = 2;
constexpr auto crotchetsPerSample =
    static_cast<float>(crotchetsPerSecond) / samplesPerSecond;

namespace fs = std::filesystem;

TEST(ProcessCallbackDrivenPlayhead, stuff) {
  auto writer = testUtils::WavFileWriter(
      fs::path{testUtils::getOutDir() + "metronome.wav"});
  ProcessCallbackDrivenPlayhead sut{samplesPerSecond, crotchetsPerSample};
  std::vector<float> metronome(2 * samplesPerSecond);
  sut.mixMetronome(metronome.data(), 2 * samplesPerSecond);
  writer.write(metronome);
}

} // namespace saint
