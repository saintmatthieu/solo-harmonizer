#include "SoloHarmonizerEditor.h"

namespace saint {
namespace {
constexpr auto chooseFileButtonTxt = "Choose MIDI file ...";
}

SoloHarmonizerEditor::SoloHarmonizerEditor(juce::AudioProcessor &p,
                                           saint::IGuiListener &guiListener)
    : AudioProcessorEditor(&p), _guiListener(guiListener),
      _chooseFileButton(chooseFileButtonTxt),
      _chooseFileButtonDefaultColour(
          _chooseFileButton.findColour(juce::TextButton::buttonColourId)) {

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 300);

  _comboBoxes[(int)TrackType::played].setTextWhenNothingSelected(
      "set play track");
  _comboBoxes[(int)TrackType::harmony].setTextWhenNothingSelected(
      "set harmonization track");
  for (auto i = 0u; i < numTrackTypes; ++i) {
    auto &box = _comboBoxes[i];
    box.setTextWhenNoChoicesAvailable("no MIDI tracks to choose from");
    box.setTooltip("setTooltip");
    box.setJustificationType(juce::Justification::centred |
                             juce::Justification::horizontallyCentred);
    box.onChange = [&box, i, this]() {
      const auto selectedTrack = box.getSelectedId();
      const auto trackType = static_cast<TrackType>(i);
      _guiListener.onTrackSelected(trackType, selectedTrack);
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
      _trackNames = _guiListener.onMidiFileChosen(path);
      _chooseFileButton.setButtonText(path.filename().string());
      _updateButtons();
    }
  };
  addAndMakeVisible(_chooseFileButton);
}

void SoloHarmonizerEditor::_updateButtons() {
  if (!_trackNames.empty()) {
    _chooseFileButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                                juce::Colours::darkgreen);
  } else {
    _chooseFileButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                                _chooseFileButtonDefaultColour);
  }
  for (auto &box : _comboBoxes) {
    box.clear();
    for (auto i = 0u; i < _trackNames.size(); ++i) {
      const auto name =
          _trackNames[i].name.empty()
              ? std::string{"Track "} + std::to_string(i + 1)
              : std::to_string(i + 1) + " : " + _trackNames[i].name;
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
  grid.templateRows = {Track(1_fr), Track(1_fr)};
  grid.templateColumns = {Track(1_fr), Track(1_fr)};
  grid.autoColumns = Track(1_fr);
  grid.autoRows = Track(1_fr);
  grid.items.addArray({GridItem(_chooseFileButton).withColumn({1, 3}),
                       GridItem(_comboBoxes[0]), GridItem(_comboBoxes[1])});
  grid.performLayout(getLocalBounds());
}
} // namespace saint
