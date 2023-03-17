#pragma once

#include "DisplayComponent.h"
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
  static constexpr auto width = 800;
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
  void onIntervalSpansAvailable(const std::vector<IntervalSpan> &) override;

  // juce::TextEditor::Listener
  void textEditorReturnKeyPressed(juce::TextEditor &) override;
  void textEditorFocusLost(juce::TextEditor &) override;

  void _onTextEditorChange(juce::TextEditor &);
  void _updateTimeSpans(const std::vector<IntervalSpan> &);
  void _updateWidgets();
  void _updatePlayButton();
  void _updateLayout();

  struct RoundedPosition {
    int barIndex;
    int beatIndex;
    bool operator==(const RoundedPosition &other) const;
    RoundedPosition &operator=(const RoundedPosition &other);
  };

  SoloHarmonizerVst &_soloHarmonizerVst;
  MidiFileOwner &_midiFileOwner;
  std::optional<juce::TextButton> _playButton;
  juce::TextButton _chooseFileButton;
  std::array<juce::ComboBox, numTrackTypes> _comboBoxes;
  const juce::Colour _chooseFileButtonDefaultColour;
  std::optional<RoundedPosition> _previousPosition;
  juce::TextEditor _barNumberDisplay;
  juce::TextEditor _beatNumberDisplay;
  juce::TextEditor _loopBeginBarEditor;
  juce::TextEditor _loopEndBarEditor;
  DisplayComponent _displayComponent;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerEditor)
};
} // namespace saint
