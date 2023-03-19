#pragma once

#include "CommonTypes.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace saint {
class DisplayComponent : public juce::Component {
public:
  DisplayComponent(const juce::Colour &backgroundColour);

  void setTimeSpans(std::vector<IntervalSpan>);
  void updateTimeInCrotchets(float);

private:
  void paint(juce::Graphics &) override;
  void resized() override;

  void _updatePixPerNote();
  float _getNoteY(int noteNumber) const;
  void _drawNoteRect(juce::Graphics &g, const juce::Rectangle<float> &rect,
                     const juce::Colour &fillColour) const;

private:
  struct Ambitus {
    int maxNote;
    int minNote;
  };
  const juce::Colour _backgroundColour;
  const juce::Colour _playedColour;
  const juce::Colour _harmoColour;
  std::vector<IntervalSpan> _spans;
  std::vector<IntervalSpan>::const_iterator _first;
  std::vector<IntervalSpan>::const_iterator _last;
  std::optional<Ambitus> _ambitus;
  float _pixPerCrot = 0.f;
  float _pixPerNote = 0.f;
  float _width = 0.f;
  float _height = 0.f;
  float _timeInCrotchets = 0.f;
};
} // namespace saint
