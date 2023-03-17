#pragma once

#include "MidiFileOwner.h"
#include "SoloHarmonizerTypes.h"
#include "SoloHarmonizerVst.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <functional>

namespace saint {
class SoloHarmonizerEditor : public juce::AudioProcessorEditor,
                             public juce::TextEditor::Listener,
                             public MidiFileOwner::Listener {
public:
  static constexpr auto width = 400;
  static constexpr auto height = 300;

  SoloHarmonizerEditor(SoloHarmonizerVst &, MidiFileOwner &);
  ~SoloHarmonizerEditor() override;

  // AudioProcessingEditor
  void paint(juce::Graphics &) override;
  void resized() override;

  void updateTimeInCrotchets(float);
  void play();

private:
  // MidiFileOwner::Listener
  void onStateChange() override;

  // juce::TextEditor::Listener
  void textEditorReturnKeyPressed(juce::TextEditor &) override;
  void textEditorFocusLost(juce::TextEditor &) override;

  void _onTextEditorChange(juce::TextEditor &);

  void _updateWidgets();
  void _updatePlayButton();

  struct RoundedPosition {
    int barIndex;
    int beatIndex;
    bool operator==(const RoundedPosition &other) const;
    RoundedPosition &operator=(const RoundedPosition &other);
  };

  SoloHarmonizerVst &_soloHarmonizerVst;
  MidiFileOwner &_midiFileOwner;
  juce::TextButton _chooseFileButton;
  juce::TextButton _playButton;
  std::array<juce::ComboBox, numTrackTypes> _comboBoxes;
  const juce::Colour _chooseFileButtonDefaultColour;
  std::optional<RoundedPosition> _previousPosition;
  juce::TextButton _barNumberDisplay;
  juce::TextButton _beatNumberDisplay;
  juce::TextEditor _loopBeginBarEditor;
  juce::TextEditor _loopEndBarEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerEditor)
};
} // namespace saint
