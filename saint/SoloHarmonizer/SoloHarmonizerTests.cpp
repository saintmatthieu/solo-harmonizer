#include "DefaultMidiFileOwner.h"
#include "Playheads/ProcessCallbackDrivenPlayhead.h"
#include "SoloHarmonizer.h"
#include "Utils.h"
#include "testUtils.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <unordered_map>

namespace saint {

namespace fs = std::filesystem;

constexpr auto blockSize = 512;
constexpr auto sampleRate = 44100;
const fs::path basePath{"C:/Users/saint/Downloads"};

void prependDelay(std::vector<float> &vector) {
  constexpr auto delayMs =
      -100; // Tempo is 4 quavers per second, i.e. 250ms. 100ms is a bit less
  constexpr auto delaySamples = delayMs * sampleRate / 1000;
  if (delaySamples >= 0) {
    const auto prevSize = vector.size();
    vector.resize(vector.size() + delaySamples);
    std::fill(vector.begin() + prevSize, vector.end(), 0.f);
    std::rotate(vector.begin(), vector.begin() + prevSize, vector.end());
  } else {
    const auto numToErase =
        std::min(static_cast<size_t>(-delaySamples), vector.size());
    vector.erase(vector.begin(), vector.begin() + numToErase);
  }
}

TEST(SoloHarmonizerTest, Les_Petits_Poissons) {
  auto wav = testUtils::fromWavFile(
      fs::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  prependDelay(wav);
  OnCrotchetsPerSecondAvailable onCrotchetsPerSecondAvailable = [](float) {};
  OnPlayheadCommand onPlayheadCommand = [](PlayheadCommand) { return false; };
  const auto factory = std::make_shared<DefaultMidiFileOwner>(
      onCrotchetsPerSecondAvailable, onPlayheadCommand);
  factory->setSampleRate(sampleRate);
  factory->setMidiFile(fs::absolute("./saint/_assets/Les_Petits_Poissons.mid"));
  factory->setPlayedTrack(1);
  factory->setHarmonyTrack(2);
  ProcessCallbackDrivenPlayhead playhead{
      sampleRate, utils::getCrotchetsPerSample(
                      *factory->getCrotchetsPerSecond(), sampleRate)};
  SoloHarmonizer sut{factory, playhead};
  sut.prepareToPlay(sampleRate, blockSize);
  for (auto offset = 0; offset + blockSize < static_cast<int>(wav.size());
       offset += blockSize) {
    const std::chrono::milliseconds now(1000 * offset / sampleRate);
    sut.processBlock(now, wav.data() + offset, blockSize);
    playhead.incrementSampleCount(blockSize);
  }
  testUtils::toWavFile(
      wav.data(), wav.size(),
      fs::path{basePath}.append("Les_Petits_Poissons_harmonized.wav"));
}
} // namespace saint
