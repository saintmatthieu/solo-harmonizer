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
      _wavFilePath = fileChooser.getResult().getFullPathName().toStdString();
      _playButton.setEnabled(true);
    }
  };

  _playButton.onClick = [this]() {
    if (!_wavFilePath.has_value()) {
      return;
    }
    auto wavReader = testUtils::WavFileReader(*_wavFilePath);
    _prepareToPlay(wavReader.getSampleRate(), 512);
    if (_playbackThread && _playbackThread->joinable()) {
      _interruptPlayback = true;
      _playbackThread->join();
    }
    _interruptPlayback = false;
    _playbackThread = std::make_unique<std::thread>([&]() {
      const auto sampleRate = wavReader.getSampleRate();
      const auto msPerBlock =
          static_cast<int>(1000.f * 512.f / static_cast<float>(sampleRate));
      while (!_interruptPlayback && wavReader.getNumSamplesAvailable() > 0) {
        std::vector<float> audio(512);
        wavReader.read(audio.data(), 512);
        _process(audio);
        std::this_thread::sleep_for(std::chrono::milliseconds(msPerBlock));
      }
    });
  };
}
} // namespace saint