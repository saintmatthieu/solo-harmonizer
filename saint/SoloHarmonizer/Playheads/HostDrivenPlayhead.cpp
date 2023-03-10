#include "HostDrivenPlayhead.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace saint {
HostDrivenPlayhead::HostDrivenPlayhead(
    const JuceAudioPlayHeadProvider &playheadProvider)
    : _playheadProvider(playheadProvider) {}

std::optional<float> HostDrivenPlayhead::getTimeInCrotchets() const {
  const juce::AudioPlayHead *playhead =
      _playheadProvider.getJuceAudioPlayHead();
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