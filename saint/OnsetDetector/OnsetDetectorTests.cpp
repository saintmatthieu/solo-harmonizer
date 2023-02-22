#include "OnsetDetector.h"

#include "pffft.hpp"
#include "testUtils.h"

#include <filesystem>
#include <gtest/gtest.h>

namespace saint {

namespace {
std::vector<float> makeNyquistWave(int numSamples) {
  std::vector<float> nyquist(numSamples);
  for (auto i = 0; i < numSamples; ++i) {
    nyquist[i] = i % 2 == 0 ? 1 : -1;
  }
  return nyquist;
}
} // namespace

TEST(OnsetDetector, firstPfftBinIsDcAndNyquist) {
  OnsetDetector sut(44100);
  constexpr auto blockSize = 512;
  const auto audio = makeNyquistWave(blockSize);
  pffft::Fft<float> fftEngine(blockSize);
  std::vector<pffft::Fft<float>::Complex> fft(blockSize / 2);
  fftEngine.forward(audio.data(), fft.data());
  EXPECT_FLOAT_EQ(fft.data()[0].real(), 0);
  EXPECT_FLOAT_EQ(fft.data()[0].imag(), blockSize);
}

TEST(OnsetDetector, stuff) {
  OnsetDetector sut(44100);
  const auto audio = testUtils::fromWavFile(
      std::filesystem::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  std::vector<float> clicks(audio.size());
  std::fill(clicks.begin(), clicks.end(), 0.f);
  constexpr auto blockSize = 512;
  for (auto n = 0u; n + blockSize < audio.size(); n += blockSize) {
    if (sut.process(audio.data() + n, blockSize)) {
      clicks[n] = 1.f;
    }
  }
  testUtils::toWavFile(
      clicks.data(), clicks.size(),
      std::filesystem::path{"C:/Users/saint/Downloads/clicks.wav"});
}

} // namespace saint
