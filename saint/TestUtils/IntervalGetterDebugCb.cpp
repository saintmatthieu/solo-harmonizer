#include "IntervalGetterDebugCb.h"

#include "WavFileWriter.h"
#include "testUtils.h"

namespace saint {
namespace testUtils {
IntervalGetterDebugCb getIntervalGetterDebugCb(int ticksPerSample) {
  const auto inputPitchWriter =
      std::make_shared<WavFileWriter>(getOutDir() + "ig_inputPitch.wav");
  const auto newIndexWriter =
      std::make_shared<WavFileWriter>(getOutDir() + "ig_newIndex.wav");
  const auto tickIntervalWriter =
      std::make_shared<WavFileWriter>(getOutDir() + "ig_tickIntervals.wav");
  const auto returnedIntervalWriter =
      std::make_shared<WavFileWriter>(getOutDir() + "ig_returnedInterval.wav");
  return [ticksPerSample, inputPitchWriter, newIndexWriter, tickIntervalWriter,
          returnedIntervalWriter, intervalIndex = 0, first = true,
          newIndexValue = -1.f](const IntervalGetterDebugCbArgs &args) mutable {
    if (first) {
      first = false;
      const auto &ticks = args.intervalTicks;
      const auto numSamples =
          static_cast<int>(ticks[ticks.size() - 1] / ticksPerSample) + 1;
      std::vector<float> changes(numSamples);
      auto value = 1.f;
      for (const auto tick : ticks) {
        const auto tickIndex = static_cast<int>(tick / ticksPerSample);
        changes[tickIndex] = value;
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