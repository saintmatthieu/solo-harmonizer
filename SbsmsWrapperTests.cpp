#include "SbsmsWrapper.h"
#include "testUtils.h"

#include <gtest/gtest.h>

TEST(SbsmsWrapperTest, DISABLED_DoesStuff) {
  auto audio = testUtils::fromWavFile();
  SbsmsWrapper sut(audio);
  const auto blockSize = sut.getInputFrameSize();
  // auto audio = makeCosine(3, blockSize * 2);
  auto i = 0u;
  sut.setPitchShift(1.f);
  while (i < audio.size()) {
    const auto numSamplesToRead = std::min(blockSize, audio.size() - i);
    sut.get(audio.data() + i, (long)numSamplesToRead);
    i += numSamplesToRead;
  }
  testUtils::toWavFile(audio.data(), audio.size());
}