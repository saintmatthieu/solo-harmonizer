#pragma once

#include "Factory/IntervalGetterFactory.h"
#include "Playheads/IPlayhead.h"
#include "SoloHarmonizer.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <memory>

namespace spdlog {
class logger;
}

namespace saint {
class IntervalGetter;

class SoloHarmonizerVst : public juce::AudioProcessor, public IPlayhead {
public:
  SoloHarmonizerVst();

  // Kept public for testing
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
  juce::AudioProcessorEditor *createEditor() override;

  // IPlayhead
  std::optional<double> getTimeInCrotchets() const override;

private:
  void releaseResources() override;
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  using AudioProcessor::processBlock;

  bool hasEditor() const override { return true; }

  const juce::String getName() const override;

  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return ""; }
  void changeProgramName(int, const juce::String &) override {}

  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

private:
  const std::shared_ptr<IntervalGetterFactory> _intervalGetterFactory;
  const std::unique_ptr<SoloHarmonizer> _soloHarmonizer;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerVst)
};
} // namespace saint
