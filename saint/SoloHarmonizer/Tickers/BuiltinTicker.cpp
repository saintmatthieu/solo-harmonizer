#include "BuiltinTicker.h"

namespace saint {
BuiltinTicker::BuiltinTicker(const AudioConfig &config)
    : _ticksPerSample(
          static_cast<double>(config.ticksPerCrotchet) *
          (config.crotchetsPerSecond == 0 ? 120 : config.crotchetsPerSecond) /
          config.samplesPerSecond) {}

void BuiltinTicker::incrementSampleCount(int numSamples) {
  _sampleCount += numSamples;
}

std::optional<int> BuiltinTicker::getTick() const {
  return static_cast<int>(_ticksPerSample * _sampleCount + 0.5);
}
} // namespace saint