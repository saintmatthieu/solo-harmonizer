#include "OnsetDetector.h"

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
  auto src = testUtils::getWavFileReader(
      fs::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  auto autoCorDst =
      testUtils::getWavFileWriter({"C:/Users/saint/Downloads/autoCor.wav"});
  auto autoCorMax =
      testUtils::getWavFileWriter({"C:/Users/saint/Downloads/autoCorMax.wav"});
  constexpr auto blockSize = 512;
  // src->readSamples(int *const *destChannels, int numDestChannels, int
  // startOffsetInDestBuffer, int64 startSampleInFile, int numSamples)
  for (auto n = 0; n + blockSize < src->lengthInSamples; n += blockSize) {
    std::vector<float> buffer(blockSize);
    std::vector<float *> channels(1);
    channels[0] = buffer.data();
    if (!src->read(channels.data(), 1, n, blockSize)) {
      ASSERT_TRUE(false);
    }
    sut.process(buffer.data(), blockSize);
    channels[0] = sut._timeData.value.data();
    autoCorDst->writeFromFloatArrays(channels.data(), 1, blockSize);
    std::fill(buffer.begin(), buffer.end(), sut._peakMax);
    channels[0] = buffer.data();
    autoCorMax->writeFromFloatArrays(channels.data(), 1, blockSize);
  }
}

} // namespace saint
