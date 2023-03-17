#include "SoloHarmonizerEditor.h"
#include "PositionGetter.h"
#include "SoloHarmonizerTypes.h"

namespace saint {
namespace {
constexpr auto chooseFileButtonTxt = "Choose MIDI file ...";
constexpr auto playTxt = "Play";
constexpr auto stopTxt = "Stop";

void initBarInputEditor(juce::TextEditor &editor,
                        const std::string &textToShowWhenEmpty,
                        juce::TextEditor::Listener *listener) {
  editor.setTextToShowWhenEmpty(textToShowWhenEmpty,
                                juce::Colours::whitesmoke.withAlpha(0.5f));
  editor.setInputFilter(
      new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789"),
      true);
  editor.addListener(listener);
}
} // namespace

SoloHarmonizerEditor::SoloHarmonizerEditor(SoloHarmonizerVst &soloHarmonizerVst,
                                           MidiFileOwner &midiFileOwner)
    : AudioProcessorEditor(&soloHarmonizerVst),
      _soloHarmonizerVst(soloHarmonizerVst), _midiFileOwner(midiFileOwner),
      _chooseFileButton(chooseFileButtonTxt), _playButton(playTxt),
      _chooseFileButtonDefaultColour(
          _chooseFileButton.findColour(juce::TextButton::buttonColourId)),
      _loopBeginBarEditor("loopBeginBarEditor"),
      _loopEndBarEditor("loopEndBarEditor") {

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(width, height);

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
        _midiFileOwner.setPlayedTrack(selectedTrack);
      } else {
        _midiFileOwner.setHarmonyTrack(selectedTrack);
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
      _midiFileOwner.setMidiFile(path);
      _updateWidgets();
    }
  };
  addAndMakeVisible(_chooseFileButton);

  _playButton.onClick = [this]() {
    if (_playButton.getButtonText() == playTxt &&
        _midiFileOwner.execute(PlayheadCommand::play)) {
      _playButton.setButtonText(stopTxt);
    } else if (_midiFileOwner.execute(PlayheadCommand::stop)) {
      _playButton.setButtonText(playTxt);
    }
  };
  addChildComponent(_playButton);

  initBarInputEditor(_loopBeginBarEditor, "Loop begin (bar)", this);
  initBarInputEditor(_loopEndBarEditor, "Loop end (bar)", this);
  addAndMakeVisible(_loopBeginBarEditor);
  addAndMakeVisible(_loopEndBarEditor);

  _barNumberDisplay.setEnabled(false);
  _beatNumberDisplay.setEnabled(false);

  addAndMakeVisible(_barNumberDisplay);
  addAndMakeVisible(_beatNumberDisplay);

  _updateWidgets();

  _midiFileOwner.setStateChangeListener(this);
}

SoloHarmonizerEditor::~SoloHarmonizerEditor() {
  _soloHarmonizerVst.onEditorDestruction(this);
  _midiFileOwner.setStateChangeListener(nullptr);
}

void SoloHarmonizerEditor::onStateChange() { _updateWidgets(); }

void SoloHarmonizerEditor::textEditorReturnKeyPressed(
    juce::TextEditor &editor) {
  _onTextEditorChange(editor);
}

void SoloHarmonizerEditor::textEditorFocusLost(juce::TextEditor &editor) {
  _onTextEditorChange(editor);
}

void SoloHarmonizerEditor::_onTextEditorChange(juce::TextEditor &editor) {
  const auto name = editor.getName();
  try {
    const auto valueStr = editor.getTextValue().toString().toStdString();
    const auto barNumber = std::stoi(valueStr);
    if (name == "loopBeginBarEditor") {
      _midiFileOwner.setLoopBeginBar(barNumber);
    } else if (name == "loopEndBarEditor") {
      _midiFileOwner.setLoopEndBar(barNumber);
    } else {
      assert(false);
    }
  } catch (...) {
    // TODO handle
  }
}

void SoloHarmonizerEditor::_updateWidgets() {
  const auto midiFilePath = _midiFileOwner.getMidiFile();
  const auto trackNames = _midiFileOwner.getMidiFileTrackNames();
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
  if (const auto playedTrack = _midiFileOwner.getPlayedTrack()) {
    _comboBoxes[playedTrackTypeIndex].setSelectedId(*playedTrack);
  }
  if (const auto harmonyTrack = _midiFileOwner.getHarmonyTrack()) {
    _comboBoxes[harmonyTrackTypeIndex].setSelectedId(*harmonyTrack);
  }

  const auto loopBeginBar = _midiFileOwner.getLoopBeginBar();
  _loopBeginBarEditor.setText(loopBeginBar ? std::to_string(*loopBeginBar)
                                           : "");
  const auto loopEndBar = _midiFileOwner.getLoopEndBar();
  _loopEndBarEditor.setText(loopEndBar ? std::to_string(*loopEndBar) : "");

  _updatePlayButton();
}

void SoloHarmonizerEditor::_updatePlayButton() {
  _playButton.setVisible(_midiFileOwner.getMidiFile().has_value() &&
                         _midiFileOwner.getPlayedTrack().has_value() &&
                         _midiFileOwner.getHarmonyTrack().has_value());
}

bool SoloHarmonizerEditor::RoundedPosition::operator==(
    const RoundedPosition &other) const {
  return std::tie(barIndex, beatIndex) ==
         std::tie(other.barIndex, other.beatIndex);
}

SoloHarmonizerEditor::RoundedPosition &
SoloHarmonizerEditor::RoundedPosition::operator=(const RoundedPosition &other) {
  barIndex = other.barIndex;
  beatIndex = other.beatIndex;
  return *this;
}

void SoloHarmonizerEditor::updateTimeInCrotchets(float crotchets) {
  if (!_midiFileOwner.hasPositionGetter()) {
    return;
  }
  const auto positionGetter = _midiFileOwner.getPositionGetter();
  if (!positionGetter) {
    return;
  }
  const auto position = positionGetter->getPosition(crotchets);
  const RoundedPosition roundedPosition{position.barIndex,
                                        static_cast<int>(position.beatIndex)};
  if (_previousPosition == roundedPosition) {
    return;
  }
  _barNumberDisplay.setButtonText(std::to_string(roundedPosition.barIndex + 1));
  _beatNumberDisplay.setButtonText(
      std::to_string(roundedPosition.beatIndex + 1));
  _previousPosition = roundedPosition;
  repaint();
}

void SoloHarmonizerEditor::play() { _playButton.triggerClick(); }

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
  grid.templateRows = {Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr)};
  grid.templateColumns = {Track(1_fr), Track(1_fr)};
  grid.autoColumns = Track(1_fr);
  grid.autoRows = Track(1_fr);
  grid.items.addArray(
      {GridItem(_chooseFileButton).withColumn({1, 3}), GridItem(_comboBoxes[0]),
       GridItem(_comboBoxes[1]), GridItem(_playButton).withColumn({1, 3}),
       GridItem(_loopBeginBarEditor), GridItem(_loopEndBarEditor),
       GridItem(_barNumberDisplay), GridItem(_beatNumberDisplay)});
  grid.performLayout(getLocalBounds());
}
} // namespace saint
