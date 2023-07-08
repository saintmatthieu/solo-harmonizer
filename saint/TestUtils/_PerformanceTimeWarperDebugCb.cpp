#include "PerformanceTimeWarperDebugCb.h"
#include "Utils.h"
#include "WavFileWriter.h"
#include "testUtils.h"

#include <filesystem>

namespace saint {
namespace testUtils {

// These functions should be called from the same thread, so no locks.
namespace {
std::unique_ptr<PerformanceTimeWarperDebugCb> cb = nullptr;
}

void setPerformanceTimeWarperDebugCbParams(int samplesPerBlock,
                                           float crotchetsPerSecond) {
  if (utils::getEnvironmentVariableAsBool(
          "SAINT_DEBUG_PERFORMANCE_TIME_WARPER") &&
      utils::isDebugBuild()) {
    const auto pitchWriter = std::make_shared<WavFileWriter>(
        std::filesystem::path{getOutDir() + "ptw_pitch.wav"});
    const auto delayCompensationWriter = std::make_shared<WavFileWriter>(
        std::filesystem::path{getOutDir() + "ptw_delayCompensation.wav"});
    cb = std::make_unique<PerformanceTimeWarperDebugCb>(
        [samplesPerBlock, pitchWriter, delayCompensationWriter,
         crotchetsPerSecond](float timeInCrotchets,
                             const std::optional<float> &pitch,
                             float warpedTime) {
          pitchWriter->write(pitch.has_value() ? *pitch / 1000.f : 0.f,
                             samplesPerBlock);
          delayCompensationWriter->write((warpedTime - timeInCrotchets) /
                                             crotchetsPerSecond,
                                         samplesPerBlock);
        });
  }
}

void resetPerformanceTimeWarperDebugCb() { cb.reset(); }

PerformanceTimeWarperDebugCb *getPerformanceTimeWarperDebugCb() {
  return cb.get();
}
} // namespace testUtils
} // namespace saint