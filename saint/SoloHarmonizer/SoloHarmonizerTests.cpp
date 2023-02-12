#include "SoloHarmonizerProcessor.h"
#include "rubberband/RubberBandStretcher.h"
#include "testUtils.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <unordered_map>

using RbStretcher = RubberBand::RubberBandStretcher;
namespace fs = std::filesystem;

constexpr auto blockSize = 512;
constexpr auto sampleRate = 44100;
const fs::path basePath{"C:/Users/saint/Downloads"};

class MockAudioPlayhead : public juce::AudioPlayHead {
public:
  MockAudioPlayhead(int ticksPerCrotchet)
      : _ticksPerSample(static_cast<double>(ticksPerCrotchet) *
                        _crotchetsPerSecond / sampleRate) {
    _positionInfo.setPpqPosition(0.0);
  }

  void advance() {
    _samplePosition += blockSize;
    _positionInfo.setPpqPosition(_samplePosition * _ticksPerSample);
  }

  juce::Optional<PositionInfo> getPosition() const { return _positionInfo; }

private:
  static constexpr int _crotchetsPerSecond =
      2; // Les Petits Poissons was recorded at 120bpm.
  const double _ticksPerSample;
  PositionInfo _positionInfo;
  int _samplePosition = 0;
};

TEST(SoloHarmonizerTest, Les_Petits_Poissons) {
  juce::MessageManager::getInstance();
  juce::MessageManagerLock lock;

  const auto input = testUtils::fromWavFile(
      fs::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  auto output = input;
  SoloHarmonizerProcessor sut{std::nullopt};
  auto ticksPerCrotchet = 0;
  sut.loadConfigFile(fs::absolute("./saint/_assets/Les_Petits_Poissons.xml"),
                     &ticksPerCrotchet);
  const auto playhead = std::make_shared<MockAudioPlayhead>(ticksPerCrotchet);
  sut.setCustomPlayhead(playhead);
  sut.prepareToPlay(sampleRate, blockSize);
  juce::MidiBuffer mbuff;
  juce::AudioBuffer<float> abuff{1, blockSize};
  for (auto offset = 0; offset + blockSize < input.size();
       offset += blockSize) {
    auto p = abuff.getWritePointer(0);
    std::copy(input.data() + offset, input.data() + offset + blockSize, p);
    sut.processBlock(abuff, mbuff);
    playhead->advance();
    std::copy(p, p + blockSize, output.data() + offset);
  }
  testUtils::toWavFile(
      output.data(), output.size(),
      fs::path{basePath}.append("Les_Petits_Poissons_harmonized.wav"));
}

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
      const auto semitoneShift =
          (float)semitoneTimeMap[index].first * (1 - fraction) +
          (float)semitoneTimeMap[index].second * fraction;
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