#include "DefaultIntervalGetterFactory.h"
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
#include <regex>
#include <string>
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

std::vector<float> getNewIndexStartTimes(const fs::path &filePath) {
  std::ifstream file{filePath};
  std::vector<float> startTimes;
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);
    const std::regex expr{"[0-9]+(\\.[0-9]+)?"};
    std::smatch match;
    std::regex_search(line, match, expr);
    try {
      startTimes.push_back(std::stof(match[0]));
    } catch (...) {
      return startTimes;
    }
  }
  return startTimes;
}

struct TestConfig {
  std::optional<float> melodyRecognizerObservationLikelihoodWeight;
};

auto runTest(const std::string &testName, TestConfig config) {
  const auto root = "C:/Users/saint/git/github/saintmatthieu/"
                    "solo-harmonizer/"s;
  const auto stem = root + "saint/_assets/"s + testName;

  const auto groundTruthFile = stem + "-index-ground-truths.txt";
  const auto startTimes = getNewIndexStartTimes(groundTruthFile);
  auto startTimeIt = startTimes.begin();

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
      onCrotchetsPerSecondAvailable, onPlayheadCommand,
      std::make_unique<DefaultIntervalGetterFactory>(
          config.melodyRecognizerObservationLikelihoodWeight));
  factory->setSampleRate(sampleRate);
  factory->setMidiFile(stem + ".mid");
  factory->setPlayedTrack(1);
  factory->setHarmonyTrack(2);
  ProcessCallbackDrivenPlayhead playhead{
      sampleRate, utils::getCrotchetsPerSample(
                      *factory->getCrotchetsPerSecond(), sampleRate)};
  SoloHarmonizer sut{factory, playhead};
  sut.prepareToPlay(sampleRate, blockSize);
  size_t numGuesses = 0;
  size_t numWrongGuesses = 0;
  for (auto offset = 0; offset + blockSize < static_cast<int>(wav.size());
       offset += blockSize) {
    const std::chrono::milliseconds now(1000 * offset / sampleRate);
    std::optional<size_t> melodyRecognizerDebugOut;
    sut.processBlock(now, wav.data() + offset, blockSize,
                     melodyRecognizerDebugOut);
    if (melodyRecognizerDebugOut.has_value()) {
      const auto time =
          static_cast<float>(offset) / static_cast<float>(sampleRate);
      auto nextStartTimeIt = std::next(startTimeIt);
      while (nextStartTimeIt != startTimes.end() && *nextStartTimeIt <= time) {
        ++nextStartTimeIt;
      }
      startTimeIt = std::prev(nextStartTimeIt);
      const auto truth = std::distance(startTimes.begin(), startTimeIt);
      ++numGuesses;
      if (truth != *melodyRecognizerDebugOut) {
        ++numWrongGuesses;
      }
    }
    playhead.incrementSampleCount(blockSize);
  }
  const auto errorPct = static_cast<float>(numWrongGuesses) * 100.f /
                        static_cast<float>(numGuesses);
  constexpr auto refErrorPct = 5.16795874f;
  ASSERT_GE(refErrorPct, errorPct);
  constexpr auto writeWav = false;
  if (writeWav) {
    testUtils::toWavFile(wav.data(), wav.size(),
                         fs::path{basePath}.append(testName + ".wav"),
                         sampleRate);
  }
  ASSERT_TRUE(true);
}

TEST(SoloHarmonizerTest, Les_Petits_Poissons) {
  const auto xml = juce::XmlDocument::parse(
      juce::File{"C:/Users/saint/Downloads/SoloHarmonizerTests.xml"});
  const auto samplesXml = xml->getChildByAttribute("name", "samples");
  const auto coefsXml = xml->getChildByName("coefs");
  const auto weightXml =
      coefsXml->getChildByAttribute("name", "observationLlhWeight");
  std::vector<float> observationLlhWeights;
  for (auto weight : weightXml->getChildIterator()) {
    observationLlhWeights.push_back(
        weight->getFirstChildElement()->getText().getFloatValue());
  }
  for (auto sample : samplesXml->getChildIterator()) {
    const auto testCase =
        sample->getFirstChildElement()->getText().toStdString();
    for (auto weight : observationLlhWeights) {
      runTest(testCase, TestConfig{weight});
    }
  }
}
} // namespace saint
