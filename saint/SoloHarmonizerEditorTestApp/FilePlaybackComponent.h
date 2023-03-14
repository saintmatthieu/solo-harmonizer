#pragma once

#include "WavFileReader.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace saint {
class FilePlaybackComponent : public juce::Component {
public:
  using PrepareToPlayFun = std::function<void(int sampleRate, int blockSize)>;
  using ProcessFun = std::function<void(std::vector<float> &)>;
  static constexpr auto height = 100;
  FilePlaybackComponent(PrepareToPlayFun, ProcessFun);

private:
  const PrepareToPlayFun _prepareToPlay;
  const ProcessFun _process;
  juce::TextButton _chooseFileButton;
  juce::TextButton _playButton;
  std::optional<std::filesystem::path> _wavFilePath;
  std::unique_ptr<std::thread> _playbackThread;
  bool _interruptPlayback = false;
};
} // namespace saint
