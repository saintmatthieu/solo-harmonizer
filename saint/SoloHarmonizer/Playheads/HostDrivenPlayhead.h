#pragma once

#include "../JuceAudioPlayHeadProvider.h"
#include "../Playhead.h"

namespace saint {
class HostDrivenPlayhead : public Playhead {
public:
  HostDrivenPlayhead(const JuceAudioPlayHeadProvider &);
  std::optional<float> getTimeInCrotchets() const override;

private:
  const JuceAudioPlayHeadProvider &_playheadProvider;
};

} // namespace saint
