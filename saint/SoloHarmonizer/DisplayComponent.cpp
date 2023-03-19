#include "DisplayComponent.h"
#include "CommonTypes.h"
#include "DisplayComponentHelper.h"

#include <cassert>
#include <limits>
#include <xlocale>

namespace saint {
DisplayComponent::DisplayComponent(const juce::Colour &backgroundColour)
    : _backgroundColour(backgroundColour),
      _playedColour(juce::Colours::olivedrab.withAlpha(0.5f)),
      _harmoColour(juce::Colours::peru.withAlpha(0.5f)) {}

void DisplayComponent::setTimeSpans(std::vector<IntervalSpan> spans) {
  _spans = spans;
  for (const auto &span : _spans) {
    if (!span.playedNote.has_value()) {
      continue;
    }
    const auto &played = *span.playedNote;
    const auto playedNoteNumber = played.noteNumber;
    if (!_ambitus.has_value()) {
      _ambitus = {playedNoteNumber, playedNoteNumber};
    }
    _ambitus->minNote = std::min(_ambitus->minNote, playedNoteNumber);
    _ambitus->maxNote = std::max(_ambitus->maxNote, playedNoteNumber);
    if (!played.interval.has_value()) {
      continue;
    }
    const auto harmonyNoteNumber = playedNoteNumber + *played.interval;
    _ambitus->minNote = std::min(_ambitus->minNote, harmonyNoteNumber);
    _ambitus->maxNote = std::max(_ambitus->maxNote, harmonyNoteNumber);
  }
  _first = _spans.begin();
  _last = _spans.begin();
  _updatePixPerNote();
}

void DisplayComponent::updateTimeInCrotchets(float timeInCrotchets) {
  if (timeInCrotchets < _timeInCrotchets) {
    _first = _spans.begin();
    _last = _spans.begin();
  }
  _timeInCrotchets = timeInCrotchets;
}

void DisplayComponent::resized() {
  _width = static_cast<float>(getWidth());
  _height = static_cast<float>(getHeight());
  _pixPerCrot = _width / widthInCrotchets;
  _updatePixPerNote();
}

void DisplayComponent::_updatePixPerNote() {
  if (!_ambitus.has_value()) {
    return;
  }
  const auto ambitus =
      static_cast<float>(_ambitus->maxNote - _ambitus->minNote);
  assert(ambitus >= 0);
  _pixPerNote =
      _height / (ambitus + 3.f); // height unit ; note height ; pixels per note
}

void DisplayComponent::_drawNoteRect(juce::Graphics &g,
                                     const juce::Rectangle<float> &rect,
                                     const juce::Colour &fillColour) const {
  constexpr auto lineThickness = 2.f;
  g.setColour(fillColour);
  g.fillRect(rect);
  g.setColour(_backgroundColour);
  g.drawRect(rect, lineThickness);
}

float DisplayComponent::_getNoteY(int noteNumber) const {
  const auto noteOffset =
      static_cast<float>(noteNumber - _ambitus->minNote + 2);
  return _height - noteOffset * _pixPerNote;
}

void DisplayComponent::paint(juce::Graphics &g) {
  g.fillAll(_backgroundColour);
  g.setColour(juce::Colours::whitesmoke);
  g.drawLine(_width / 2, 0, _width / 2, _height);
  if (!_ambitus.has_value()) {
    return;
  }
  const auto ambitus =
      static_cast<float>(_ambitus->maxNote - _ambitus->minNote);
  assert(ambitus >= 0);
  setToImageWidth(_spans, _first, _last, _timeInCrotchets);
  const auto crotchetOffset = _timeInCrotchets - widthInCrotchets / 2;
  for (auto it = _first; it != _last; ++it) {
    if (!it->playedNote.has_value()) {
      continue;
    }
    const auto &played = it->playedNote;
    const auto jt = std::next(it);
    const auto xLeft = (it->beginCrotchet - crotchetOffset) * _pixPerCrot;
    const auto xRight =
        jt == _spans.end() ? _width
                           : (jt->beginCrotchet - crotchetOffset) * _pixPerCrot;
    const auto playedY = _getNoteY(played->noteNumber);
    const juce::Rectangle<float> playedRect = {xLeft, playedY, xRight - xLeft,
                                               _pixPerNote};
    _drawNoteRect(g, playedRect, _playedColour);
    if (!played->interval.has_value()) {
      continue;
    }
    const auto interval = *played->interval;
    const auto harmoY = _getNoteY(played->noteNumber + interval);
    const juce::Rectangle<float> harmoRect = {xLeft, harmoY, xRight - xLeft,
                                              _pixPerNote};
    _drawNoteRect(g, harmoRect, _harmoColour);
  }
}
} // namespace saint
