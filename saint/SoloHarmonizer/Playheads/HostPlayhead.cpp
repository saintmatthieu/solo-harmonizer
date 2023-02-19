#include "HostPlayhead.h"

namespace saint {
std::optional<double> HostPlayhead::getTimeInCrotchets() const {
  const juce::AudioPlayHead *playhead = _getPlayHead();
  if (!playhead) {
    // TODO log
    return std::nullopt;
  }
  const auto position = playhead->getPosition();
  if (!position) {
    return std::nullopt;
  }
  const auto ppq = position->getPpqPosition();
  if (!ppq) {
    // TODO log
    return std::nullopt;
  }
  return *ppq;
}
} // namespace saint