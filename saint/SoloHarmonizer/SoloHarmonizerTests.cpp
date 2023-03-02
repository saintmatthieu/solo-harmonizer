#include "Factory/IntervalGetterFactory.h"
#include "Playheads/BuiltinPlayhead.h"
#include "SoloHarmonizer.h"
#include "testUtils.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <unordered_map>

namespace fs = std::filesystem;

constexpr auto blockSize = 512;
constexpr auto sampleRate = 44100;
const fs::path basePath{"C:/Users/saint/Downloads"};

namespace saint {
TEST(SoloHarmonizerTest, Les_Petits_Poissons) {
  auto wav = testUtils::fromWavFile(
      fs::absolute("./saint/_assets/Les_Petits_Poissons.wav"));
  const auto factory = std::make_shared<IntervalGetterFactory>();
  factory->setMidiFile(fs::absolute("./saint/_assets/Les_Petits_Poissons.mid"));
  factory->setPlayedTrack(1);
  factory->setHarmonyTrack(2);
  AudioConfig config;
  config.samplesPerSecond = sampleRate;
  config.crotchetsPerSecond = *factory->getCrotchetsPerSecond();
  BuiltinPlayhead playhead{config};
  SoloHarmonizer sut{factory, playhead};
  sut.prepareToPlay(sampleRate, blockSize);
  for (auto offset = 0; offset + blockSize < static_cast<int>(wav.size());
       offset += blockSize) {
    sut.processBlock(wav.data() + offset, blockSize);
    playhead.incrementSampleCount(blockSize);
  }
  testUtils::toWavFile(
      wav.data(), wav.size(),
      fs::path{basePath}.append("Les_Petits_Poissons_harmonized.wav"));
}
} // namespace saint
