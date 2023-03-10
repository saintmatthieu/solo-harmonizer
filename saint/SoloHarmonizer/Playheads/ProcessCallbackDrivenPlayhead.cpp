#include "ProcessCallbackDrivenPlayhead.h"
#include <optional>

namespace saint {
ProcessCallbackDrivenPlayhead::ProcessCallbackDrivenPlayhead(
    float crotchetsPerSample)
    : _crotchetsPerSample(crotchetsPerSample) {}

void ProcessCallbackDrivenPlayhead::incrementSampleCount(int numSamples) {
  if (!_paused) {
    _sampleCount += numSamples;
  }
}

bool ProcessCallbackDrivenPlayhead::execute(PlayheadCommand command) {
  switch (command) {
  case PlayheadCommand::play:
    _paused = false;
    break;
  case PlayheadCommand::pause:
    _paused = true;
    break;
  case PlayheadCommand::stop:
    _paused = true;
    _sampleCount = 0;
    break;
  }
  return true;
}

std::optional<float> ProcessCallbackDrivenPlayhead::getTimeInCrotchets() const {
  if (_paused) {
    return std::nullopt;
  } else {
    return static_cast<float>(_sampleCount.load()) * _crotchetsPerSample;
  }
}
} // namespace saint