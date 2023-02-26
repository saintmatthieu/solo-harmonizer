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

struct WavWriters {
  std::unique_ptr<juce::AudioFormatWriter> autoCorr;
  std::unique_ptr<juce::AudioFormatWriter> autoCorrMax;
};

WavWriters makeWriters(int analysisIndex) {
  auto autoCor = testUtils::getWavFileWriter(
      {std::string("C:/Users/saint/Downloads/autoCor_" +
                   std::to_string(analysisIndex) + ".wav")});
  auto autoCorMax = testUtils::getWavFileWriter(
      {std::string("C:/Users/saint/Downloads/autoCorMax_" +
                   std::to_string(analysisIndex) + ".wav")});
  return {std::move(autoCor), std::move(autoCorMax)};
}

TEST(OnsetDetector, stuff) {
  // src->readSamples(int *const *destChannels, int numDestChannels, int
  // startOffsetInDestBuffer, int64 startSampleInFile, int numSamples)
  std::array<WavWriters, 2> debugWriters{makeWriters(0), makeWriters(1)};
  auto first = true;
  OnXcorReady debugCallback = [&debugWriters,
                               &first](const std::vector<float> &xcorr,
                                       int windowSize, int peakIndex,
                                       int ringBufferIndex, float scaledMax) {
    std::vector<float> truncatedXcorr(windowSize);
    std::vector<const float *> channels(1);
    std::copy(xcorr.begin(), xcorr.begin() + windowSize / 2,
              truncatedXcorr.begin());
    std::copy(xcorr.end() - windowSize / 2, xcorr.end(),
              truncatedXcorr.begin() + windowSize / 2);
    truncatedXcorr[peakIndex] = 0.f;
    const auto offset = first ? windowSize / 2 : 0;
    const auto size = first ? windowSize / 2 : windowSize;
    first = false;
    channels[0] = truncatedXcorr.data() + offset;
    debugWriters[ringBufferIndex].autoCorr->writeFromFloatArrays(
        channels.data(), 1, size);
    std::vector<float> buffer(size);
    std::fill(buffer.begin(), buffer.end(), scaledMax);
    channels[0] = buffer.data();
    debugWriters[ringBufferIndex].autoCorrMax->writeFromFloatArrays(
        channels.data(), 1, size);
  };

  const auto src = testUtils::getWavFileReader(
      "C:/Users/saint/Downloads/TOP-80-GREATEST-GUITAR-INTROS.wav");
  // const auto src = testUtils::getWavFileReader(
  //     fs::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  constexpr auto blockSize = 512;
  OnsetDetector sut(44100, std::move(debugCallback));
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
