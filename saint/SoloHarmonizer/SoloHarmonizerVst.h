#pragma once

#include "JuceAudioPlayHeadProvider.h"
#include "MidiFileOwner.h"
#include "Playhead.h"
#include "SoloHarmonizer.h"
#include "SoloHarmonizerTypes.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <filesystem>
#include <memory>
#include <thread>
#include <unordered_set>

namespace spdlog {
class logger;
}

namespace saint {
class Clock {
public:
  virtual std::chrono::milliseconds now() const = 0;
  virtual ~Clock() = default;
};

class SoloHarmonizerVst : public juce::AudioProcessor,
                          public Playhead,
                          public MidiFileOwner::Listener,
                          JuceAudioPlayHeadProvider {
public:
  SoloHarmonizerVst(PlayheadFactory);
  ~SoloHarmonizerVst() override;

  const bool isStandalone;

  // Kept public for testing
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
  void onEditorDestruction(SoloHarmonizerEditor *);
  SoloHarmonizerEditor *createSoloHarmonizerEditor();

  // Playhead
  std::optional<float> incrementSampleCount(int) override;
  std::optional<float> getTimeInCrotchets() override;

  // JuceAudioPlayHeadProvider
  juce::AudioPlayHead *getJuceAudioPlayHead() const override;

private:
  // MidiFileOwner::Listener
  void onLoopBeginBarChange(const std::optional<int> &) override;
  void onLoopEndBarChange(const std::optional<int> &) override;

  juce::AudioProcessorEditor *createEditor() override;
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
  void _onCrotchetsPerSecondAvailable(float);
  bool _onPlayheadCommand(PlayheadCommand);
  bool _startPlaying();
  bool _stopPlaying();
  void _editorCallThreadFun();
  std::atomic<std::optional<float>> _timeInCrotchets;
  std::atomic<std::optional<int>> _loopBeginBar;
  std::atomic<std::optional<int>> _loopEndBar;
  std::optional<float> _crotchetsPerSecond;
  std::optional<int> _samplesPerSecond;
  const std::shared_ptr<MidiFileOwner> _midiFileOwner;
  const std::unique_ptr<SoloHarmonizer> _soloHarmonizer;
  const std::unique_ptr<Clock> _clock;
  const PlayheadFactory _playheadFactory;
  std::shared_ptr<Playhead> _playhead;
  std::unordered_set<SoloHarmonizerEditor *> _editors;
  std::thread _editorCallThread;
  bool _runEditorCallThread = true;
  std::mutex _editorMutex;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerVst)
};
} // namespace saint
