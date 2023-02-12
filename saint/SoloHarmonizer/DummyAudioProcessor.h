#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class DummyAudioProcessor : public juce::AudioProcessor {
private:
  void prepareToPlay(double, int) override {}
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override {}
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout &) const override {
    return true;
  }
  juce::AudioProcessorEditor *createEditor() override { return nullptr; }
  bool hasEditor() const override { return false; }
  const juce::String getName() const override { return "DummyAudioProcessor"; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return ""; }
  void changeProgramName(int, const juce::String &) override {}
  void getStateInformation(juce::MemoryBlock &) override {}
  void setStateInformation(const void *, int) override {}
};
