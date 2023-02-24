#include "OnsetDetector.h"

#include "pffft.hpp"
#include "testUtils.h"

#include <filesystem>
#include <gtest/gtest.h>

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
  auto audio = testUtils::fromWavFile(
      fs::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  // std::fill(audio.begin(), audio.end(), 1.f);
  std::vector<float> autoCor(audio.size());
  std::vector<float> autoCorMax(audio.size());
  constexpr auto blockSize = 512;
  for (auto n = 0u; n + blockSize < audio.size(); n += blockSize) {
    sut.process(audio.data() + n, blockSize);
    std::copy(sut._timeData.value.begin(), sut._timeData.value.end(),
              autoCor.begin() + n);
    std::fill(autoCorMax.begin() + n, autoCorMax.begin() + n + blockSize,
              sut._peakMax);
    autoCor[n + sut._peakMaxIndex] = -1;
  }
  testUtils::toWavFile(autoCor.data(), autoCor.size(),
                       fs::path{"C:/Users/saint/Downloads/autoCor.wav"});
  testUtils::toWavFile(autoCorMax.data(), autoCorMax.size(),
                       fs::path{"C:/Users/saint/Downloads/autoCorMax.wav"});
}

} // namespace saint
