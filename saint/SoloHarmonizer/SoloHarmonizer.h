#pragma once

#include "DavidCNAntonia/PitchShifter.h"
#include "Factory/EditorsFactoryView.h"
#include "Factory/ProcessorsFactoryView.h"
#include "Playheads/IPlayhead.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <libpyincpp.h>

#include <filesystem>
#include <memory>

namespace spdlog {
class logger;
}

namespace saint {
class HarmoPitchGetter;

class SoloHarmonizer : public juce::AudioProcessor {
public:
  SoloHarmonizer(std::optional<RubberBand::RubberBandStretcher::Options> opts,
                 std::shared_ptr<EditorsFactoryView>,
                 std::shared_ptr<ProcessorsFactoryView>);
  ~SoloHarmonizer() override;

  // For testing
  void setSemitoneShift(float value);

  // Kept public for testing
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

private:
  void releaseResources() override;
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor *createEditor() override;
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
  const std::optional<RubberBand::RubberBandStretcher::Options>
      _rbStretcherOptions;
  const std::shared_ptr<EditorsFactoryView> _editorsFactoryView;
  const std::shared_ptr<ProcessorsFactoryView> _processorsFactoryView;
  const std::string _loggerName;
  const std::shared_ptr<spdlog::logger> _logger;
  std::unique_ptr<DavidCNAntonia::PitchShifter> _pitchShifter;
  const std::unique_ptr<IPlayhead> _playhead;

  // For testing
  std::optional<int> _ticksPerCrotchet = std::nullopt;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizer)
};
} // namespace saint
