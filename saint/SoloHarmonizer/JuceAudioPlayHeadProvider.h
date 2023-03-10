#pragma once

namespace juce {
class AudioPlayHead;
}

namespace saint {
class JuceAudioPlayHeadProvider {
public:
  virtual ~JuceAudioPlayHeadProvider() = default;
  virtual juce::AudioPlayHead *getJuceAudioPlayHead() const = 0;
};
} // namespace saint
