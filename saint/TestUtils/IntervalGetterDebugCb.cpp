#include "IntervalGetterDebugCb.h"

#include "testUtils.h"

namespace saint {
namespace testUtils {
IntervalGetterDebugCb getIntervalGetterDebugCb() {
  const auto inputPitchWriter =
      std::make_shared<WavFileWriter>(getOutDir() + "ig_inputPitch.wav");
  const auto newIndexWriter =
      std::make_shared<WavFileWriter>(getOutDir() + "ig_newIndex.wav");
  return [inputPitchWriter,
          newIndexWriter](const IntervalGetterDebugCbArgs &args) {
    inputPitchWriter->write(args.inputPitch ? *args.inputPitch / 1000 : 0.f,
                            args.blockSize);
    newIndexWriter->write(args.newIndex / static_cast<float>(args.numIndices),
                          args.blockSize);
  };
}
} // namespace testUtils
} // namespace saint