#pragma once

#include "LibPyin/source/libpyincpp.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <rubberband/RubberBandStretcher.h>

//==============================================================================
class SoloHarmonizerProcessor : public juce::AudioProcessor,
                                public juce::FileBrowserListener {
public:
  //==============================================================================
  SoloHarmonizerProcessor();
  ~SoloHarmonizerProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
  using AudioProcessor::processBlock;

  //==============================================================================
  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  // File browser listener
  void selectionChanged() override {}
  void fileClicked(const juce::File &, const juce::MouseEvent &) override {}
  void fileDoubleClicked(const juce::File &file) override;
  void browserRootChanged(const juce::File &) override {}

private:
  juce::WildcardFileFilter _fileFilter;
  juce::FileBrowserComponent _fileBrowserComponent;
  std::unique_ptr<PyinCpp> _pyinCpp;
  std::unique_ptr<RubberBand::RubberBandStretcher> _stretcher;
  juce::Label _pitchDisplay;

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerProcessor)
};
