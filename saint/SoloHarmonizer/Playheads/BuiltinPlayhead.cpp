#include "BuiltinPlayhead.h"

namespace saint {
BuiltinPlayhead::BuiltinPlayhead(const AudioConfig &config)
    : _ticksPerSample(
          static_cast<double>(config.ticksPerCrotchet) *
          (config.crotchetsPerSecond == 0 ? 120 : config.crotchetsPerSecond) /
          config.samplesPerSecond) {}

void BuiltinPlayhead::incrementSampleCount(int numSamples) {
  _sampleCount += numSamples;
}

std::optional<double> BuiltinPlayhead::getTimeInCrotchets() const {
  return _ticksPerSample * _sampleCount;
}
} // namespace saint