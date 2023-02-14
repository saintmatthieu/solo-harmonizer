#include "HostTicker.h"

namespace saint {
std::optional<int> HostTicker::getTick() const {
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
  // TODO is it wrong to round to int ?..
  return static_cast<int>(*ppq + 0.5);
}
} // namespace saint