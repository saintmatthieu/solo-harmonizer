#include "SoloHarmonizerEditor.h"
#include "PositionGetter.h"
#include "SoloHarmonizerTypes.h"

namespace saint {
namespace {
constexpr auto chooseFileButtonTxt = "Choose MIDI file ...";
constexpr auto playTxt = "Play";
constexpr auto stopTxt = "Stop";
constexpr auto middle = juce::Justification::verticallyCentred |
                        juce::Justification::horizontallyCentred;
enum class ReadOnly { no, yes };
void initBarBeatDisplay(juce::TextEditor &editor,
                        const std::string &textToShowWhenEmpty,
                        ReadOnly readOnly,
                        juce::TextEditor::Listener *listener = nullptr) {
  editor.setTextToShowWhenEmpty(textToShowWhenEmpty,
                                juce::Colours::whitesmoke.withAlpha(0.5f));
  editor.setInputFilter(
      new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789"),
      true);
  if (listener) {
    editor.addListener(listener);
  }
  editor.setReadOnly(readOnly == ReadOnly::yes);
  editor.setJustification(middle);
}
} // namespace

SoloHarmonizerEditor::SoloHarmonizerEditor(SoloHarmonizerVst &soloHarmonizerVst,
                                           MidiFileOwner &midiFileOwner)
    : AudioProcessorEditor(&soloHarmonizerVst),
      _soloHarmonizerVst(soloHarmonizerVst), _midiFileOwner(midiFileOwner),
      _playButton(soloHarmonizerVst.isStandalone
                      ? std::optional<juce::TextButton>{playTxt}
                      : std::nullopt),
      _chooseFileButton(chooseFileButtonTxt),
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
    box.setJustificationType(middle);
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

  if (_playButton) {
    _playButton->onClick = [this]() {
      if (_playButton->getButtonText() == playTxt &&
          _midiFileOwner.execute(PlayheadCommand::play)) {
        _playButton->setButtonText(stopTxt);
      } else if (_midiFileOwner.execute(PlayheadCommand::stop)) {
        _playButton->setButtonText(playTxt);
      }
    };
    addChildComponent(*_playButton);
  }

  initBarBeatDisplay(_loopBeginBarEditor, "Loop begin (bar)", ReadOnly::no,
                     this);
  initBarBeatDisplay(_loopEndBarEditor, "Loop end (bar)", ReadOnly::no, this);
  initBarBeatDisplay(_barNumberDisplay, "bar number", ReadOnly::yes);
  initBarBeatDisplay(_beatNumberDisplay, "beat number", ReadOnly::yes);

  addAndMakeVisible(_loopBeginBarEditor);
  addAndMakeVisible(_loopEndBarEditor);
  addAndMakeVisible(_barNumberDisplay);
  addAndMakeVisible(_beatNumberDisplay);
  addAndMakeVisible(_displayComponent);

  _updateWidgets();

  _midiFileOwner.addStateChangeListener(this);
  if (const auto spans = _midiFileOwner.getIntervalSpans()) {
    _updateTimeSpans(*spans);
  }
}

SoloHarmonizerEditor::~SoloHarmonizerEditor() {
  _soloHarmonizerVst.onEditorDestruction(this);
  _midiFileOwner.addStateChangeListener(nullptr);
}

void SoloHarmonizerEditor::onStateChange() { _updateWidgets(); }

void SoloHarmonizerEditor::onIntervalSpansAvailable(
    const std::vector<IntervalSpan> &spans) {
  _updateTimeSpans(spans);
}

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
    const std::optional<int> barNumber =
        valueStr.empty() ? std::optional<int>{} : std::stoi(valueStr);
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

void SoloHarmonizerEditor::_updateTimeSpans(
    const std::vector<IntervalSpan> &spans) {
  juce::MessageManager::getInstance()->callAsync([spans, this]() {
    _displayComponent.setTimeSpans(spans);
    _updateLayout();
  });
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
  if (!_playButton) {
    return;
  }
  _playButton->setVisible(_midiFileOwner.getMidiFile().has_value() &&
                          _midiFileOwner.getPlayedTrack().has_value() &&
                          _midiFileOwner.getHarmonyTrack().has_value());
}

void SoloHarmonizerEditor::_updateLayout() {
  using namespace juce; // for _px, _fr, ...;
  Grid grid;
  grid.rowGap = 10_px;
  grid.columnGap = 10_px;
  using Track = Grid::TrackInfo;
  grid.templateRows = {Track(1_fr), Track(1_fr), Track(1_fr)};
  if (_playButton) {
    grid.templateRows.add(Track(1_fr));
    grid.items.add(GridItem(*_playButton).withRow({4, 5}).withColumn({1, 5}));
  }
  grid.templateColumns = {Track(1_fr), Track(1_fr), Track(1_fr), Track(1_fr),
                          Track(4_fr)};
  grid.items.addArray(
      {GridItem(_chooseFileButton).withRow({1, 2}).withColumn({1, 5}),
       GridItem(_comboBoxes[0]).withRow({2, 3}).withColumn({1, 3}),
       GridItem(_comboBoxes[1]).withRow({2, 3}).withColumn({3, 5}),
       GridItem(_loopBeginBarEditor).withRow({3, 4}).withColumn({1, 2}),
       GridItem(_loopEndBarEditor).withRow({3, 4}).withColumn({2, 3}),
       GridItem(_barNumberDisplay).withRow({3, 4}).withColumn({3, 4}),
       GridItem(_beatNumberDisplay).withRow({3, 4}).withColumn({4, 5}),
       GridItem(_displayComponent).withRow({1, 6}).withColumn({5, 6})});
  grid.performLayout(getLocalBounds());
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
  _previousPosition = roundedPosition;
  const auto barNumberStr = std::to_string(roundedPosition.barIndex + 1);
  const auto beatNumberStr = std::to_string(roundedPosition.beatIndex + 1);
  juce::MessageManager::getInstance()->callAsync(
      [this, barNumberStr, beatNumberStr, crotchets]() {
        _barNumberDisplay.setText(barNumberStr);
        _beatNumberDisplay.setText(beatNumberStr);
        _displayComponent.updateTimeInCrotchets(crotchets);
        repaint();
      });
}

void SoloHarmonizerEditor::play() {
  if (!_playButton) {
    return;
  }
  _playButton->triggerClick();
}

void SoloHarmonizerEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SoloHarmonizerEditor::resized() { _updateLayout(); }
} // namespace saint
