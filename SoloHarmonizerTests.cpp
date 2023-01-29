#include "SoloHarmonizerProcessor.h"
#include "testUtils.h"

#include <algorithm>
#include <gtest/gtest.h>

TEST(SoloHamonizerTest, DoesStuff) {
  juce::MessageManager::getInstance();
  juce::MessageManagerLock lock;
  SoloHarmonizerProcessor sut;
  constexpr auto blockSize = 512;
  sut.prepareToPlay(44100, blockSize);
  std::vector<float> wav = testUtils::fromWavFile();
  juce::MidiBuffer mbuff;
  juce::AudioBuffer<float> abuff{1, blockSize};
  for (auto offset = 0; offset + blockSize < wav.size(); offset += blockSize) {
    auto p = abuff.getWritePointer(0);
    std::copy(wav.data() + offset, wav.data() + offset + blockSize, p);
    sut.processBlock(abuff, mbuff);
    std::copy(p, p + blockSize, wav.data() + offset);
  }
  testUtils::toWavFile(wav.data(), wav.size());
}