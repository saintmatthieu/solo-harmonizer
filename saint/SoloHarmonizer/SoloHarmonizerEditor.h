#pragma once

#include "IGuiListener.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <functional>

namespace saint {
class SoloHarmonizerEditor : public juce::AudioProcessorEditor {
public:
  explicit SoloHarmonizerEditor(juce::AudioProcessor &, IGuiListener &);

  // AudioProcessingEditor
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  void _updateButtons();

  IGuiListener &_guiListener;
  juce::TextButton _chooseFileButton;
  std::array<juce::ComboBox, numTrackTypes> _comboBoxes;
  std::vector<TrackInfo> _trackNames;

  const juce::Colour _chooseFileButtonDefaultColour;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerEditor)
};
} // namespace saint
