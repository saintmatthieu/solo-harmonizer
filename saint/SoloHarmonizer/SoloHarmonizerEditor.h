#pragma once

#include "Intervaller/EditorsFactoryView.h"
#include "SoloHarmonizerTypes.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <functional>

namespace saint {
class SoloHarmonizerEditor : public juce::AudioProcessorEditor {
public:
  SoloHarmonizerEditor(juce::AudioProcessor &, EditorsFactoryView &);

  // AudioProcessingEditor
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  void _updateWidgets();

  EditorsFactoryView &_intervallerFactoryView;
  juce::TextButton _chooseFileButton;
  std::array<juce::ComboBox, numTrackTypes> _comboBoxes;
  juce::ToggleButton _useHostPlayheadToggle;

  const juce::Colour _chooseFileButtonDefaultColour;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerEditor)
};
} // namespace saint
