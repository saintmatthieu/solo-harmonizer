#include "DisplayComponent.h"

namespace saint {
DisplayComponent::DisplayComponent(std::vector<IntervalSpan> spans)
    : _spans(spans) {}

void DisplayComponent::updateTimeInCrotchets(float timeInCrotchets) {
  _timeInCrotchets = timeInCrotchets;
}

void DisplayComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::beige);
}
} // namespace saint
