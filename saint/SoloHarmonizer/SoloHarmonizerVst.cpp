#include "SoloHarmonizerVst.h"
#include "Playheads/HostDrivenPlayhead.h"
#include "Playheads/ProcessCallbackDrivenPlayhead.h"
#include "SoloHarmonizerEditor.h"
#include "Utils.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <thread>

namespace saint {

using namespace std::placeholders;

SoloHarmonizerVst::SoloHarmonizerVst(PlayheadFactory factory)
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::mono(), true)
              .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      _isStandalone(wrapperType ==
                    juce::AudioProcessor::wrapperType_Standalone),
      _midiFileOwner(std::make_shared<DefaultMidiFileOwner>(
          std::bind(&SoloHarmonizerVst::_onCrotchetsPerSecondAvailable, this,
                    _1),
          std::bind(&SoloHarmonizerVst::_onPlayheadCommand, this, _1))),
      _soloHarmonizer(std::make_unique<SoloHarmonizer>(_midiFileOwner, *this)),
      _playheadFactory(std::move(factory)),
      _editorCallThread(
          std::bind(&SoloHarmonizerVst::_editorCallThreadFun, this)) {}

SoloHarmonizerVst::~SoloHarmonizerVst() {
  _runEditorCallThread = false;
  _editorCallThread.join();
}

void SoloHarmonizerVst::_editorCallThreadFun() {
  while (_runEditorCallThread) {
    const auto time = _timeInCrotchets.load();
    if (time.has_value()) {
      std::lock_guard<std::mutex> lock(_editorMutex);
      for (auto editor : _editors) {
        editor->updateTimeInCrotchets(*time);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{50});
  }
}

const juce::String SoloHarmonizerVst::getName() const {
  return JucePlugin_Name;
}

void SoloHarmonizerVst::prepareToPlay(double sampleRate, int samplesPerBlock) {
  _samplesPerSecond = static_cast<int>(sampleRate);
  _midiFileOwner->setSampleRate(*_samplesPerSecond);
  _soloHarmonizer->prepareToPlay(*_samplesPerSecond, samplesPerBlock);
}

void SoloHarmonizerVst::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  _soloHarmonizer->releaseResources();
}

bool SoloHarmonizerVst::isBusesLayoutSupported(const BusesLayout &) const {
  return true;
}

void SoloHarmonizerVst::processBlock(juce::AudioBuffer<float> &buffer,
                                     juce::MidiBuffer &) {
  const auto playhead = _playhead;
  const auto numSamples = buffer.getNumSamples();
  if (playhead) {
    const auto p = buffer.getWritePointer(0);
    _soloHarmonizer->processBlock(p, numSamples);
    playhead->mixMetronome(p, numSamples);
    playhead->incrementSampleCount(numSamples);
    _timeInCrotchets = playhead->getTimeInCrotchets();
  }
  const auto p = buffer.getReadPointer(0);
  for (auto i = 1; i < buffer.getNumChannels(); ++i) {
    const auto q = buffer.getWritePointer(i);
    std::copy(p, p + numSamples, q);
  }
}

std::optional<float> SoloHarmonizerVst::getTimeInCrotchets() const {
  return _timeInCrotchets;
}

juce::AudioPlayHead *SoloHarmonizerVst::getJuceAudioPlayHead() const {
  return getPlayHead();
}

SoloHarmonizerEditor *SoloHarmonizerVst::createSoloHarmonizerEditor() {
  const auto editor = new SoloHarmonizerEditor(*this, *_midiFileOwner);
  {
    std::lock_guard<std::mutex> lock(_editorMutex);
    _editors.insert(editor);
  }
  return editor;
}

juce::AudioProcessorEditor *SoloHarmonizerVst::createEditor() {
  return createSoloHarmonizerEditor();
}

void SoloHarmonizerVst::onEditorDestruction(SoloHarmonizerEditor *editor) {
  std::lock_guard<std::mutex> lock(_editorMutex);
  if (_editors.count(editor) > 0u) {
    _editors.erase(editor);
  }
}

void SoloHarmonizerVst::getStateInformation(juce::MemoryBlock &destData) {
  const auto &state = _midiFileOwner->getState();
  destData.append(state.data(), state.size());
}

void SoloHarmonizerVst::setStateInformation(const void *data, int size) {
  _midiFileOwner->setState(std::vector<char>{
      static_cast<const char *>(data), static_cast<const char *>(data) + size});
}

void SoloHarmonizerVst::_onCrotchetsPerSecondAvailable(
    float crotchetsPerSecond) {
  _crotchetsPerSecond = crotchetsPerSecond;
}

bool SoloHarmonizerVst::_onPlayheadCommand(PlayheadCommand command) {
  switch (command) {
  case PlayheadCommand::play:
    return _startPlaying();
  case PlayheadCommand::stop:
    return _stopPlaying();
  case PlayheadCommand::pause:
  default:
    // TODO: log
    return false;
  }
}

bool SoloHarmonizerVst::_startPlaying() {
  _playhead = _playheadFactory(_isStandalone, *this, _crotchetsPerSecond,
                               _samplesPerSecond);
  return _playhead != nullptr;
}

bool SoloHarmonizerVst::_stopPlaying() {
  _playhead.reset();
  return true;
}
} // namespace saint

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  using namespace saint;
  PlayheadFactory factory{[](bool mustSetPpqPosition,
                             const JuceAudioPlayHeadProvider &playheadProvider,
                             const std::optional<float> &crotchetsPerSecond,
                             const std::optional<int> &samplesPerSecond)
                              -> std::shared_ptr<Playhead> {
    if (mustSetPpqPosition) {
      if (!crotchetsPerSecond.has_value() || samplesPerSecond.has_value()) {
        return nullptr;
      }
      const auto crotchetsPerSample =
          utils::getCrotchetsPerSample(*crotchetsPerSecond, *samplesPerSecond);
      return std::make_shared<ProcessCallbackDrivenPlayhead>(
          *samplesPerSecond, crotchetsPerSample);
    } else {
      return std::make_shared<HostDrivenPlayhead>(playheadProvider);
    }
  }};
  return new SoloHarmonizerVst(std::move(factory));
}
