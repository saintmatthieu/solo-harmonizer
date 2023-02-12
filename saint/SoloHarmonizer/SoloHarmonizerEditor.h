#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <functional>

class SoloHarmonizerEditor : public juce::AudioProcessorEditor,
                             public juce::FileBrowserListener {
public:
  using LoadConfigFile = std::function<void(const std::filesystem::path &)>;

  explicit SoloHarmonizerEditor(juce::AudioProcessor &,
                                LoadConfigFile loadConfigFile);

  // AudioProcessingEditor
  void paint(juce::Graphics &) override;
  void resized() override;

  // File browser listener
  void selectionChanged() override {}
  void fileClicked(const juce::File &, const juce::MouseEvent &) override {}
  void fileDoubleClicked(const juce::File &file) override;
  void browserRootChanged(const juce::File &) override {}

private:
  const LoadConfigFile _loadConfigFile;
  juce::WildcardFileFilter _fileFilter;
  juce::FileBrowserComponent _fileBrowserComponent;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerEditor)
};
