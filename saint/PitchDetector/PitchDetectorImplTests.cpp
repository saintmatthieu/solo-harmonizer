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
  PitchDetectorImpl sut(44100);
  constexpr auto blockSize = 512;
  const auto audio = makeNyquistWave(blockSize);
  pffft::Fft<float> fftEngine(blockSize);
  std::vector<pffft::Fft<float>::Complex> fft(blockSize / 2);
  fftEngine.forward(audio.data(), fft.data());
  EXPECT_FLOAT_EQ(fft.data()[0].real(), 0);
  EXPECT_FLOAT_EQ(fft.data()[0].imag(), blockSize);
}

struct OlapMetricWriters {
  std::unique_ptr<juce::AudioFormatWriter> autoCorr;
  std::unique_ptr<juce::AudioFormatWriter> autoCorrMax;
};

struct MetricWriters {
  std::unique_ptr<juce::AudioFormatWriter> combinedMax;
  std::array<OlapMetricWriters, 2> olapWriters;
};

const std::string outDir = "C:/Users/saint/Downloads/";

OlapMetricWriters makeOlapMetricWriters(int analysisIndex) {
  auto autoCor = testUtils::getWavFileWriter(
      {outDir + "autoCor_" + std::to_string(analysisIndex) + ".wav"});
  auto autoCorMax = testUtils::getWavFileWriter(
      {outDir + "autoCorMax_" + std::to_string(analysisIndex) + ".wav"});
  return {std::move(autoCor), std::move(autoCorMax)};
}

TEST(PitchDetectorImpl, stuff) {
  constexpr auto blockSize = 512;
  MetricWriters metricWriters;
  metricWriters.combinedMax =
      testUtils::getWavFileWriter({outDir + "autoCorMaxMin.wav"});
  metricWriters.olapWriters = {makeOlapMetricWriters(0),
                               makeOlapMetricWriters(1)};
  auto first = true;
  OnXcorReady debugCallback = [&first,
                               &metricWriters](const OnXcorReadyArgs &args) {
    std::vector<float> truncatedXcorr(args.windowSize);
    std::vector<const float *> channels(1);
    std::copy(args.xcor.begin(), args.xcor.begin() + args.windowSize / 2,
              truncatedXcorr.begin());
    std::copy(args.xcor.end() - args.windowSize / 2, args.xcor.end(),
              truncatedXcorr.begin() + args.windowSize / 2);
    truncatedXcorr[std::min((size_t)args.peakIndex,
                            truncatedXcorr.size() - 1)] = 0.f;
    const auto offset = first ? args.windowSize / 2 : 0;
    const auto size = first ? args.windowSize / 2 : args.windowSize;
    first = false;
    channels[0] = truncatedXcorr.data() + offset;
    metricWriters.olapWriters[args.olapAnalIndex]
        .autoCorr->writeFromFloatArrays(channels.data(), 1, size);
    std::vector<float> buffer(size);
    std::fill(buffer.begin(), buffer.end(), args.scaledMax);
    channels[0] = buffer.data();
    metricWriters.olapWriters[args.olapAnalIndex]
        .autoCorrMax->writeFromFloatArrays(channels.data(), 1, size);
    std::fill(buffer.begin(), buffer.begin() + size / 2, args.maxMin);
    metricWriters.combinedMax->writeFromFloatArrays(channels.data(), 1,
                                                    size / 2);
  };

  const auto src = testUtils::getWavFileReader(
      "C:/Users/saint/Downloads/TOP-80-GREATEST-GUITAR-INTROS.wav");
  // const auto src = testUtils::getWavFileReader(
  //     fs::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  PitchDetectorImpl sut(44100, std::move(debugCallback));
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
