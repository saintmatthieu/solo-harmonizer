#include "DisplayComponent.h"

namespace saint {
void DisplayComponent::updateTimeInCrotchets(float timeInCrotchets) {
  _timeInCrotchets = timeInCrotchets;
}

void DisplayComponent::setTimeSpans(std::vector<IntervalSpan> spans) {
  _spans = spans;
}

void DisplayComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::beige);
}
} // namespace saint
