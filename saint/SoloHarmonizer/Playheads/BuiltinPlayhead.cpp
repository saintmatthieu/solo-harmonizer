#include "BuiltinPlayhead.h"

namespace saint {
BuiltinPlayhead::BuiltinPlayhead(const AudioConfig &config)
    : _crotchetsPerSample(
          (config.crotchetsPerSecond == 0 ? 120 : config.crotchetsPerSecond) /
          config.samplesPerSecond) {}

void BuiltinPlayhead::incrementSampleCount(int numSamples) {
  _sampleCount += numSamples;
}

std::optional<double> BuiltinPlayhead::getTimeInCrotchets() const {
  return _crotchetsPerSample * _sampleCount;
}
} // namespace saint