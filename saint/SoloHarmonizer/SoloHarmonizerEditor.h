#pragma once

#include "IGuiListener.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <functional>

class SoloHarmonizerEditor : public juce::AudioProcessorEditor {
public:
  explicit SoloHarmonizerEditor(juce::AudioProcessor &, saint::IGuiListener &);

  // AudioProcessingEditor
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  saint::IGuiListener &_guiListener;
  juce::TextButton _chooseFileButton;
  juce::ComboBox _playedTrackComboBox;
  juce::ComboBox _harmonyTrackComboBox;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerEditor)
};
