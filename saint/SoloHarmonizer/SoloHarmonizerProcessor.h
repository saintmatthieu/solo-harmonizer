#pragma once

#include "../CommonTypes.h"
#include "DavidCNAntonia/PitchShifter.h"
#include "HarmoPitchGetter.h"
#include "IGuiListener.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <libpyincpp.h>
#include <rubberband/RubberBandStretcher.h>

#include <filesystem>
#include <memory>

namespace spdlog {
class logger;
}

class SoloHarmonizerProcessor : public juce::AudioProcessor,
                                public saint::IGuiListener {
public:
  SoloHarmonizerProcessor(
      std::optional<RubberBand::RubberBandStretcher::Options> opts);
  ~SoloHarmonizerProcessor() override;

  // For testing
  void loadMidiFile(const std::filesystem::path &,
                    int *ticksPerCrotchet = nullptr);
  void setPlayedTrack(int);
  void setHarmonyTrack(int);
  void setSemitoneShift(float value);
  void setCustomPlayhead(std::weak_ptr<juce::AudioPlayHead>);

  // Kept public for testing
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

private:
  void releaseResources() override;
  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void onMidiFileChosen(const std::filesystem::path &) override;
  bool isReady() const override;
  std::set<int> getMidiTracks() const override;

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
  const juce::String getProgramName(int) override {}
  void changeProgramName(int, const juce::String &) override {}

  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

private:
  void _updatePitchEstimate(float const *, size_t);
  void _runPitchShift(juce::AudioBuffer<float> &buffer);
  std::optional<float> _getHarmonySemitones();
  void _reloadIfReady();

  const std::optional<RubberBand::RubberBandStretcher::Options>
      _rbStretcherOptions;
  const std::string _loggerName;
  const std::shared_ptr<spdlog::logger> _logger;
  std::unique_ptr<RubberBand::RubberBandStretcher> _stretcher;
  std::unique_ptr<PitchShifter> _pitchShifter;
  std::unique_ptr<PyinCpp> _pitchEstimator;
  std::optional<float> _pitchEstimate = std::nullopt;
  std::unique_ptr<saint::HarmoPitchGetter> _harmoPitchGetter;
  std::weak_ptr<juce::AudioPlayHead> _customPlayhead;
  bool _getPositionLogged = false;
  bool _getPpqPositionLogged = false;
  saint::Config _config;

  // For testing
  std::optional<int> _ticksPerCrotchet = std::nullopt;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerProcessor)
};
