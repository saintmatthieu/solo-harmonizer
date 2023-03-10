#include "ProcessCallbackDrivenPlayhead.h"
#include <optional>

namespace saint {
ProcessCallbackDrivenPlayhead::ProcessCallbackDrivenPlayhead(
    float crotchetsPerSample)
    : _crotchetsPerSample(crotchetsPerSample) {}

void ProcessCallbackDrivenPlayhead::incrementSampleCount(int numSamples) {
  _sampleCount += numSamples;
}

void ProcessCallbackDrivenPlayhead::mixMetronome(float *, int) {}

std::optional<float> ProcessCallbackDrivenPlayhead::getTimeInCrotchets() const {
  return static_cast<float>(_sampleCount) * _crotchetsPerSample;
}
} // namespace saint