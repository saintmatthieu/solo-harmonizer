#include "SoloHarmonizerEditor.h"

namespace saint {
namespace {
constexpr auto chooseFileButtonTxt = "Choose MIDI file ...";
}

SoloHarmonizerEditor::SoloHarmonizerEditor(
    juce::AudioProcessor &p, EditorsFactoryView &intervallerFactory)
    : AudioProcessorEditor(&p), _intervallerFactoryView(intervallerFactory),
      _chooseFileButton(chooseFileButtonTxt),
      _useHostPlayheadToggle("use host playhead"),
      _chooseFileButtonDefaultColour(
          _chooseFileButton.findColour(juce::TextButton::buttonColourId)) {

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 300);

  _useHostPlayheadToggle.setToggleState(
      false, juce::NotificationType::dontSendNotification);
  _useHostPlayheadToggle.onClick = []() {
    // Plug this back (maybe)
  };
  addAndMakeVisible(_useHostPlayheadToggle);

  auto &playedComboBox = _comboBoxes[(int)TrackType::played];
  auto &harmonyComboBox = _comboBoxes[(int)TrackType::harmony];
  playedComboBox.setTextWhenNothingSelected("set play track");
  harmonyComboBox.setTextWhenNothingSelected("set harmonization track");
  if (const auto playedTrack = _intervallerFactoryView.getPlayedTrack()) {
    playedComboBox.setSelectedId(*playedTrack);
  }
  if (const auto harmonyTrack = _intervallerFactoryView.getHarmonyTrack()) {
    playedComboBox.setSelectedId(*harmonyTrack);
  }
  for (auto i = 0u; i < numTrackTypes; ++i) {
    const auto trackType = static_cast<TrackType>(i);
    auto &box = _comboBoxes[i];
    box.setTextWhenNoChoicesAvailable("no MIDI tracks to choose from");
    box.setJustificationType(juce::Justification::centred |
                             juce::Justification::horizontallyCentred);
    box.onChange = [&box, trackType, this]() {
      const auto selectedTrack = box.getSelectedId();
      if (trackType == TrackType::played) {
        _intervallerFactoryView.setPlayedTrack(selectedTrack);
      } else {
        _intervallerFactoryView.setHarmonyTrack(selectedTrack);
      }
    };
    addAndMakeVisible(box);
  }

  _chooseFileButton.setTooltip(
      "MIDI file of the song whose solo you'd like to play "
      "with it harmonization !");
  _chooseFileButton.onClick = [this]() {
    juce::FileChooser fileChooser("Choose MIDI file ...", juce::File(),
                                  "*.mid;*.midi");
    if (fileChooser.browseForFileToOpen()) {
      const std::filesystem::path path =
          fileChooser.getResult().getFullPathName().toStdString();
      _intervallerFactoryView.setMidiFile(path);
      _updateWidgets();
    }
  };
  addAndMakeVisible(_chooseFileButton);
}

void SoloHarmonizerEditor::_updateWidgets() {
  const auto midiFilePath = _intervallerFactoryView.getMidiFile();
  const auto trackNames = _intervallerFactoryView.getMidiFileTrackNames();
  _chooseFileButton.setButtonText(
      midiFilePath ? midiFilePath->filename().string() : chooseFileButtonTxt);
  if (!trackNames.empty()) {
    _chooseFileButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                                juce::Colours::darkgreen);
  } else {
    _chooseFileButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                                _chooseFileButtonDefaultColour);
  }
  for (auto &box : _comboBoxes) {
    box.clear();
    for (auto i = 0u; i < trackNames.size(); ++i) {
      const auto name = trackNames[i].empty()
                            ? std::string{"Track "} + std::to_string(i + 1)
                            : std::to_string(i + 1) + " : " + trackNames[i];
      box.addItem(name, (int)i + 1);
    }
  }
}

void SoloHarmonizerEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SoloHarmonizerEditor::resized() {
  using namespace juce; // for _px, _fr, ...;
  Grid grid;
  grid.rowGap = 20_px;
  grid.columnGap = 20_px;
  using Track = Grid::TrackInfo;
  grid.templateRows = {Track(1_fr), Track(1_fr), Track(1_fr)};
  grid.templateColumns = {Track(1_fr), Track(1_fr)};
  grid.autoColumns = Track(1_fr);
  grid.autoRows = Track(1_fr);
  grid.items.addArray({GridItem(_useHostPlayheadToggle).withColumn({1, 3}),
                       GridItem(_chooseFileButton).withColumn({1, 3}),
                       GridItem(_comboBoxes[0]), GridItem(_comboBoxes[1])});
  grid.performLayout(getLocalBounds());
}
} // namespace saint
