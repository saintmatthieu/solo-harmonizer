#pragma once

#include "MidiFileOwner.h"
#include "SoloHarmonizerTypes.h"
#include "SoloHarmonizerVst.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <functional>

namespace saint {
class SoloHarmonizerEditor : public juce::AudioProcessorEditor {
public:
  SoloHarmonizerEditor(SoloHarmonizerVst &, MidiFileOwner &);
  ~SoloHarmonizerEditor() override;

  // AudioProcessingEditor
  void paint(juce::Graphics &) override;
  void resized() override;

  void updateTimeInCrotchets(float);

private:
  void _updateWidgets();
  void _updatePlayButton();

  SoloHarmonizerVst &_soloHarmonizerVst;
  MidiFileOwner &_midiFileOwner;
  juce::TextButton _chooseFileButton;
  juce::TextButton _playButton;
  std::array<juce::ComboBox, numTrackTypes> _comboBoxes;
  const juce::Colour _chooseFileButtonDefaultColour;
  std::optional<Position> _previousPosition;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerEditor)
};
} // namespace saint
