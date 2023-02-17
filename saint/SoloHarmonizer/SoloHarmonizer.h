#pragma once

#include "DavidCNAntonia/PitchShifter.h"
#include "HarmoPitchGetter.h"
#include "IGuiListener.h"
#include "SoloHarmonizerTypes.h"
#include "Tickers/ITicker.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <libpyincpp.h>
#include <rubberband/RubberBandStretcher.h>

#include <filesystem>
#include <memory>

namespace spdlog {
class logger;
}

namespace saint {
class SoloHarmonizer : public juce::AudioProcessor, public IGuiListener {
public:
  SoloHarmonizer(std::optional<RubberBand::RubberBandStretcher::Options> opts);
  ~SoloHarmonizer() override;

  void setSemitoneShift(float value);

  // Kept public for testing
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  // IGuiListener
  std::vector<TrackInfo>
  onMidiFileChosen(const std::filesystem::path &) override;
  void onTrackSelected(TrackType, int trackNumber) override;
  bool getUseHostPlayhead() const override;
  void setUseHostPlayhead(bool) override;

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
  void _runPitchShift(juce::AudioBuffer<float> &buffer);
  void _reloadIfReady();

  const std::optional<RubberBand::RubberBandStretcher::Options>
      _rbStretcherOptions;
  const std::string _loggerName;
  const std::shared_ptr<spdlog::logger> _logger;
  std::unique_ptr<RubberBand::RubberBandStretcher> _stretcher;
  std::unique_ptr<PitchShifter> _pitchShifter;
  std::unique_ptr<HarmoPitchGetter> _harmoPitchGetter;
  std::optional<juce::MidiFile> _midiFile;
  std::optional<int> _playedTrackNumber;
  std::optional<int> _harmonyTrackNumber;
  std::unique_ptr<ITicker> _ticker;
  AudioConfig _audioConfig;
  bool _useHostPlayhead = false;

  // For testing
  std::optional<int> _ticksPerCrotchet = std::nullopt;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizer)
};
} // namespace saint
