#include "SoloHarmonizerProcessor.h"
#include "rubberband/RubberBandStretcher.h"
#include "testUtils.h"

#include <__msvc_chrono.hpp>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <unordered_map>

using RbStretcher = RubberBand::RubberBandStretcher;
namespace fs = std::filesystem;

TEST(SoloHamonizerTest, Benchmarking) {
  constexpr auto baseOpts = RbStretcher::Option::OptionProcessRealTime +
                            RbStretcher::Option::OptionPitchHighConsistency +
                            RbStretcher::Option::OptionFormantShifted +
                            RbStretcher::Option::OptionTransientsSmooth +
                            RbStretcher::Option::OptionPhaseIndependent +
                            RbStretcher::Option::OptionWindowLong;
  const std::unordered_map<std::string, RbStretcher::Options> optionMap = {
      // The base options above seem to be yielding the best quality - and
      // performance !
      {"FavouriteSetting", static_cast<RbStretcher::Option>(0)},
  };

  juce::MessageManager::getInstance();
  juce::MessageManagerLock lock;
  constexpr auto blockSize = 512;
  const fs::path basePath{"C:/Users/saint/Downloads"};
  const std::string baseFilename{"rb_benchmark_"};
  const std::vector<float> input = testUtils::fromWavFile();
  const auto timingFilename =
      fs::path{basePath}.append("rb_processing_time_ms.txt");
  if (fs::exists(timingFilename)) {
    fs::remove(timingFilename);
  }
  std::ofstream ofs{timingFilename};
  const std::vector<std::pair<int, int>> semitoneTimeMap{{0, 1},   {10, 10}, //
                                                         {-2, 0},  {8, 8},   //
                                                         {-4, -2}, {6, 6},   //
                                                         {-6, -4}, {5, 5}};

  for (const auto &entry : optionMap) {
    const auto &filenameSuffix = entry.first;
    const auto &opts = entry.second;
    auto output = input;
    SoloHarmonizerProcessor sut{baseOpts + opts};
    constexpr auto sampleRate = 44100;
    sut.prepareToPlay(sampleRate, blockSize);
    juce::MidiBuffer mbuff;
    juce::AudioBuffer<float> abuff{1, blockSize};
    std::chrono::milliseconds totalTimeMs{0};
    for (auto offset = 0; offset + blockSize < input.size();
         offset += blockSize) {

      const auto playheadTimeSec = (float)offset / sampleRate;

      // interpolate pitch shift value
      const auto index = (int)playheadTimeSec;
      const auto fraction = playheadTimeSec - index;
      const auto semitoneShift = semitoneTimeMap[index].first * (1 - fraction) +
                                 semitoneTimeMap[index].second * fraction;
      sut.setSemitoneShift(semitoneShift);

      auto p = abuff.getWritePointer(0);
      std::copy(input.data() + offset, input.data() + offset + blockSize, p);
      const auto startTime = std::chrono::steady_clock::now();
      sut.processBlock(abuff, mbuff);
      const auto stopTime = std::chrono::steady_clock::now();
      totalTimeMs += std::chrono::duration_cast<std::chrono::milliseconds>(
          stopTime - startTime);
      std::copy(p, p + blockSize, output.data() + offset);
    }
    ofs << filenameSuffix << ": " << totalTimeMs.count() << std::endl;
    testUtils::toWavFile(
        output.data(), output.size(),
        fs::path{basePath}.append(baseFilename + filenameSuffix + ".wav"));
  }
}