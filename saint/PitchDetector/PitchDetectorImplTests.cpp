#include "PitchDetectorImpl.h"

#include "pffft.hpp"
#include "testUtils.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace saint {

namespace fs = std::filesystem;

namespace {
std::vector<float> makeNyquistWave(int numSamples) {
  std::vector<float> nyquist(numSamples);
  for (auto i = 0; i < numSamples; ++i) {
    nyquist[i] = i % 2 == 0 ? 1 : -1;
  }
  return nyquist;
}
} // namespace

TEST(PitchDetectorImpl, firstPfftBinIsDcAndNyquist) {
  PitchDetectorImpl sut(44100, std::nullopt, std::nullopt);
  constexpr auto blockSize = 512;
  const auto audio = makeNyquistWave(blockSize);
  pffft::Fft<float> fftEngine(blockSize);
  std::vector<pffft::Fft<float>::Complex> fft(blockSize / 2);
  fftEngine.forward(audio.data(), fft.data());
  EXPECT_FLOAT_EQ(fft.data()[0].real(), 0);
  EXPECT_FLOAT_EQ(fft.data()[0].imag(), blockSize);
}

TEST(PitchDetectorImpl, stuff) {
  const auto debugCb = testUtils::getPitchDetectorDebugCb();
  constexpr auto blockSize = 512;
  // const auto src = testUtils::getJuceWavFileReader(
  //     "C:/Users/saint/Downloads/TOP-80-GREATEST-GUITAR-INTROS.wav");
  const auto src = testUtils::getJuceWavFileReader(
      fs::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  PitchDetectorImpl sut(44100, 83.f, std::move(debugCb));
  for (auto n = 0; n + blockSize < src->lengthInSamples; n += blockSize) {
    std::vector<float> buffer(blockSize);
    std::vector<float *> channels(1);
    channels[0] = buffer.data();
    if (!src->read(channels.data(), 1, n, blockSize)) {
      ASSERT_TRUE(false);
    }
    // std::fill(buffer.begin(), buffer.end(), 1.f);
    sut.process(buffer.data(), blockSize);
  }
}

} // namespace saint
