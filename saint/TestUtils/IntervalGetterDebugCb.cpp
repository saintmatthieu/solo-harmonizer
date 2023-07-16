#include "IntervalGetterDebugCb.h"

#include "WavFileWriter.h"
#include "testUtils.h"

namespace saint {
namespace testUtils {
// I might be breaking this test code right now ...
IntervalGetterDebugCb getIntervalGetterDebugCb(float crotchetsPerSample,
                                               int sampleRate) {
  const auto inputPitchWriter = std::make_shared<WavFileWriter>(
      getOutDir() + "ig_inputPitch.wav", sampleRate);
  const auto newIndexWriter = std::make_shared<WavFileWriter>(
      getOutDir() + "ig_newIndex.wav", sampleRate);
  const auto tickIntervalWriter = std::make_shared<WavFileWriter>(
      getOutDir() + "ig_tickIntervals.wav", sampleRate);
  const auto returnedIntervalWriter = std::make_shared<WavFileWriter>(
      getOutDir() + "ig_returnedInterval.wav", sampleRate);
  return [crotchetsPerSample, inputPitchWriter, newIndexWriter,
          tickIntervalWriter, returnedIntervalWriter, intervalIndex = 0,
          first = true,
          newIndexValue = -1.f](const IntervalGetterDebugCbArgs &args) mutable {
    if (first) {
      first = false;
      const auto &crotchets = args.intervalCrotchets;
      const auto numSamples = static_cast<int>(crotchets[crotchets.size() - 1] /
                                               crotchetsPerSample) +
                              1;
      std::vector<float> changes(numSamples);
      auto value = 1.f;
      for (const auto crotchet : crotchets) {
        const auto crotchetIndex =
            static_cast<int>(crotchet / crotchetsPerSample);
        changes[crotchetIndex] = value;
        value *= -1.f;
      }
      tickIntervalWriter->write(changes);
    }
    inputPitchWriter->write(args.inputPitch ? *args.inputPitch / 1000 : 0.f,
                            args.blockSize);
    std::vector<float> newIndexVec(args.blockSize);
    if (args.newIndex != intervalIndex) {
      newIndexVec[0] = newIndexValue;
      newIndexValue *= -1.f;
      intervalIndex = args.newIndex;
    }
    newIndexWriter->write(newIndexVec);
    returnedIntervalWriter->write(
        args.returnedInterval.has_value() ? 0.5f : 0.f, args.blockSize);
  };
}
} // namespace testUtils
} // namespace saint