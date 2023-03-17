#pragma once

#include "CommonTypes.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace saint {
class DisplayComponent : public juce::Component {
public:
  DisplayComponent(std::vector<IntervalSpan>);
  void updateTimeInCrotchets(float);

private:
  void paint(juce::Graphics &) override;

private:
  const std::vector<IntervalSpan> _spans;
  float _timeInCrotchets = 0.f;
};
} // namespace saint
