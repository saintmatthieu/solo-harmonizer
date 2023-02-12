#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerProcessor.h"

//==============================================================================
SoloHarmonizerEditor::SoloHarmonizerEditor(juce::AudioProcessor &p,
                                           LoadConfigFile loadConfigFile)
    : AudioProcessorEditor(&p), _loadConfigFile(std::move(loadConfigFile)),
      _fileFilter("*.xml", "", ""),
      _fileBrowserComponent(
          juce::FileBrowserComponent::FileChooserFlags::openMode |
              juce::FileBrowserComponent::FileChooserFlags::canSelectFiles,
          juce::File(), &_fileFilter, nullptr) {
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 300);
  _fileBrowserComponent.addListener(this);
  _fileBrowserComponent.setSize(250, 250);
  addAndMakeVisible(_fileBrowserComponent);
}

//==============================================================================
void SoloHarmonizerEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void SoloHarmonizerEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
}

void SoloHarmonizerEditor::fileDoubleClicked(const juce::File &file) {
  _loadConfigFile(file.getFullPathName().toStdString());
}
