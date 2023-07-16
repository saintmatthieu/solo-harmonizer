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

#include <juce_core/juce_core.h> // XML

using namespace std::literals::string_literals;

namespace saint {

namespace fs = std::filesystem;

constexpr auto blockSize = 512;
const fs::path basePath{"C:/Users/saint/Downloads"};

void prependDelay(std::vector<float> &vector, int sampleRate) {
  constexpr auto delayMs =
      -100; // Tempo is 4 quavers per second, i.e. 250ms. 100ms is a bit less
  const auto delaySamples = delayMs * sampleRate / 1000;
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

void runTest(const std::string &testName) {
  const auto stem = "C:/Users/saint/git/github/saintmatthieu/"
                    "solo-harmonizer/saint/_assets/"s +
                    testName;
  const auto wavFileName = stem + ".wav";
  auto sampleRate = 0;
  auto wav = testUtils::fromWavFile(wavFileName, sampleRate);
  constexpr auto addDelay = false;
  if (addDelay) {
    prependDelay(wav, sampleRate);
  }
  OnCrotchetsPerSecondAvailable onCrotchetsPerSecondAvailable = [](float) {};
  OnPlayheadCommand onPlayheadCommand = [](PlayheadCommand) { return false; };
  const auto factory = std::make_shared<DefaultMidiFileOwner>(
      onCrotchetsPerSecondAvailable, onPlayheadCommand);
  factory->setSampleRate(sampleRate);
  factory->setMidiFile(stem + ".mid");
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
      fs::path{basePath}.append("Les_Petits_Poissons_harmonized.wav"),
      sampleRate);
}

TEST(SoloHarmonizerTest, Les_Petits_Poissons) {
  const auto xml = juce::XmlDocument::parse(
      juce::File{"C:/Users/saint/Downloads/SoloHarmonizerTests.xml"});
  const auto samplesXml = xml->getChildByAttribute("name", "samples");
  for (auto sample : samplesXml->getChildIterator()) {
    runTest(sample->getFirstChildElement()->getText().toStdString());
  }
}
} // namespace saint
