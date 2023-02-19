#pragma once

#include "IPlayhead.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace saint {
class HostPlayhead : public IPlayhead {
  using GetPlayHead = std::function<const juce::AudioPlayHead *()>;

public:
  HostPlayhead(GetPlayHead getPlayHead)
      : _getPlayHead(std::move(getPlayHead)) {}
  std::optional<double> getTimeInCrotchets() const override;

private:
  GetPlayHead _getPlayHead;
};
} // namespace saint
