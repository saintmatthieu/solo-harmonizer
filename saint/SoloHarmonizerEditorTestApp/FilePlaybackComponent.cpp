#include "FilePlaybackComponent.h"
#include "SoloHarmonizerEditor.h"
#include "WavFileReader.h"
#include <thread>

namespace saint {
FilePlaybackComponent::FilePlaybackComponent(PrepareToPlayFun prepareToPlay,
                                             ProcessFun process)
    : _prepareToPlay(prepareToPlay), _process(process),
      _chooseFileButton("Choose wav file ..."), _playButton("Play") {
  setSize(SoloHarmonizerEditor::width, height);
  _chooseFileButton.setSize(SoloHarmonizerEditor::width / 2, height);
  _playButton.setSize(SoloHarmonizerEditor::width / 2, height);
  _playButton.setTopLeftPosition(SoloHarmonizerEditor::width / 2, 0);
  _playButton.setEnabled(false);
  addAndMakeVisible(_chooseFileButton);
  addAndMakeVisible(_playButton);

  _chooseFileButton.onClick = [this]() {
    juce::FileChooser fileChooser("Choose wav file ...", juce::File(), "*.wav");
    if (fileChooser.browseForFileToOpen()) {
      const std::filesystem::path path =
          fileChooser.getResult().getFullPathName().toStdString();
      _wavReader.reset(new testUtils::WavFileReader(path));
      _playButton.setEnabled(true);
    }
  };

  _playButton.onClick = [this]() {
    _prepareToPlay(_wavReader->getSampleRate(), 512);
    _playbackThread = std::make_unique<std::thread>([this]() {
      const auto sampleRate = _wavReader->getSampleRate();
      const auto msPerBlock =
          static_cast<int>(1000.f * 512.f / static_cast<float>(sampleRate));
      while (_wavReader->getNumSamplesAvailable() > 0) {
        std::vector<float> audio(512);
        _wavReader->read(audio.data(), 512);
        _process(audio);
        std::this_thread::sleep_for(std::chrono::milliseconds(msPerBlock));
      }
    });
  };
}
} // namespace saint