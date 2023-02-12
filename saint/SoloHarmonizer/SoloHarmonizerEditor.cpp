#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerProcessor.h"

//==============================================================================
SoloHarmonizerEditor::SoloHarmonizerEditor(juce::AudioProcessor &p,
                                           saint::IGuiListener &guiListener)
    : AudioProcessorEditor(&p), _guiListener(guiListener),
      _chooseFileButton("Choose MIDI file ...") {

  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 300);

  _playedTrackComboBox.setTooltip("MIDI track you'll be playing");
  _harmonyTrackComboBox.setTooltip("Harmonization MIDI track");
  _playedTrackComboBox.setEnabled(false);
  _harmonyTrackComboBox.setEnabled(false);

  _chooseFileButton.setTooltip(
      "MIDI file of the song whose solo you'd like to play "
      "with it harmonization !");
  _chooseFileButton.onClick = [this]() {
    juce::FileChooser fileChooser("Choose MIDI file ...", juce::File(),
                                  "*.mid;*.midi");
    if (fileChooser.browseForFileToOpen()) {
      const std::filesystem::path path =
          fileChooser.getResult().getFullPathName().toStdString();
      _guiListener.onMidiFileChosen(path);
    }
  };
  addAndMakeVisible(_chooseFileButton);
  addAndMakeVisible(_playedTrackComboBox);
  addAndMakeVisible(_harmonyTrackComboBox);
}

//==============================================================================
void SoloHarmonizerEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SoloHarmonizerEditor::resized() {
  using namespace juce; // for _px;
  Grid grid;
  grid.rowGap = 20_px;
  grid.columnGap = 20_px;
  using Track = Grid::TrackInfo;
  grid.templateRows = {Track(1_fr), Track(1_fr)};
  grid.templateColumns = {Track(1_fr), Track(1_fr)};
  grid.autoColumns = Track(1_fr);
  grid.autoRows = Track(1_fr);
  grid.items.addArray({GridItem(_chooseFileButton).withColumn({2}),
                       GridItem(_playedTrackComboBox).withRow({2}),
                       GridItem(_harmonyTrackComboBox).withRow({2})});

  grid.performLayout(getLocalBounds());
}
