#pragma once

#include "ITicker.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace saint {
class HostTicker : public ITicker {
  using GetPlayHead = std::function<const juce::AudioPlayHead *()>;

public:
  HostTicker(GetPlayHead getPlayHead) : _getPlayHead(std::move(getPlayHead)) {}
  std::optional<int> getTick() const override;

private:
  GetPlayHead _getPlayHead;
};
} // namespace saint
