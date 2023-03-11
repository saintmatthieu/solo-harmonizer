#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerTypes.h"

namespace saint {
namespace {
constexpr auto chooseFileButtonTxt = "Choose MIDI file ...";
constexpr auto playTxt = "Play";
constexpr auto stopTxt = "Stop";
} // namespace

SoloHarmonizerEditor::SoloHarmonizerEditor(
    SoloHarmonizerVst &soloHarmonizerVst,
    EditorsFactoryView &intervallerFactory)
    : AudioProcessorEditor(&soloHarmonizerVst),
      _soloHarmonizerVst(soloHarmonizerVst),
      _intervallerFactoryView(intervallerFactory),
      _chooseFileButton(chooseFileButtonTxt), _playButton(playTxt),
      _chooseFileButtonDefaultColour(
          _chooseFileButton.findColour(juce::TextButton::buttonColourId)) {

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 300);

  _comboBoxes[playedTrackTypeIndex].setTextWhenNothingSelected(
      "set play track");
  _comboBoxes[harmonyTrackTypeIndex].setTextWhenNothingSelected(
      "set harmonization track");
  for (auto i = 0u; i < numTrackTypes; ++i) {
    const auto trackType = static_cast<TrackType>(i);
    auto &box = _comboBoxes[i];
    box.setJustificationType(juce::Justification::centred |
                             juce::Justification::horizontallyCentred);
    box.onChange = [&box, trackType, this]() {
      const auto selectedTrack = box.getSelectedId();
      if (trackType == TrackType::played) {
        _intervallerFactoryView.setPlayedTrack(selectedTrack);
      } else {
        _intervallerFactoryView.setHarmonyTrack(selectedTrack);
      }
      _updatePlayButton();
    };
    addChildComponent(box);
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

  _playButton.onClick = [this]() {
    if (_playButton.getButtonText() == playTxt &&
        _intervallerFactoryView.execute(PlayheadCommand::play)) {
      _playButton.setButtonText(stopTxt);
    } else if (_intervallerFactoryView.execute(PlayheadCommand::stop)) {
      _playButton.setButtonText(playTxt);
    }
  };
  addChildComponent(_playButton);

  _updateWidgets();
}

void SoloHarmonizerEditor::_updateWidgets() {
  const auto midiFilePath = _intervallerFactoryView.getMidiFile();
  const auto trackNames = _intervallerFactoryView.getMidiFileTrackNames();
  _chooseFileButton.setButtonText(
      midiFilePath ? midiFilePath->filename().string() : chooseFileButtonTxt);
  _chooseFileButton.setColour(juce::TextButton::ColourIds::buttonColourId,
                              trackNames.empty()
                                  ? _chooseFileButtonDefaultColour
                                  : juce::Colours::darkgreen);
  for (auto &box : _comboBoxes) {
    box.clear();
    for (auto i = 0u; i < trackNames.size(); ++i) {
      const auto name = trackNames[i].empty()
                            ? std::string{"Track "} + std::to_string(i + 1)
                            : std::to_string(i + 1) + " : " + trackNames[i];
      box.addItem(name, (int)i + 1);
    }
    box.setVisible(!trackNames.empty());
  }
  if (const auto playedTrack = _intervallerFactoryView.getPlayedTrack()) {
    _comboBoxes[playedTrackTypeIndex].setSelectedId(*playedTrack);
  }
  if (const auto harmonyTrack = _intervallerFactoryView.getHarmonyTrack()) {
    _comboBoxes[harmonyTrackTypeIndex].setSelectedId(*harmonyTrack);
  }
  _updatePlayButton();
}

SoloHarmonizerEditor::~SoloHarmonizerEditor() {
  _soloHarmonizerVst.onEditorDestruction(this);
}

void SoloHarmonizerEditor::_updatePlayButton() {
  _playButton.setVisible(_intervallerFactoryView.getMidiFile().has_value() &&
                         _intervallerFactoryView.getPlayedTrack().has_value() &&
                         _intervallerFactoryView.getHarmonyTrack().has_value());
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
  grid.items.addArray({GridItem(_chooseFileButton).withColumn({1, 3}),
                       GridItem(_comboBoxes[0]), GridItem(_comboBoxes[1]),
                       GridItem(_playButton).withColumn({1, 3})});
  grid.performLayout(getLocalBounds());
}

void SoloHarmonizerEditor::updateTimeInCrotchets(float crotchets) {}

} // namespace saint
