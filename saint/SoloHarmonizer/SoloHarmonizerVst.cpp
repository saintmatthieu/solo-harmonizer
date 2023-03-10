#include "SoloHarmonizerVst.h"
#include "Factory/IntervalGetterFactory.h"
#include "Playheads/HostDrivenPlayhead.h"
#include "Playheads/ProcessCallbackDrivenPlayhead.h"
#include "SoloHarmonizerEditor.h"
#include "Utils.h"

#include <cassert>
#include <optional>

namespace saint {

using namespace std::placeholders;

SoloHarmonizerVst::SoloHarmonizerVst(PlayheadFactory factory)
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::mono(), true)
              .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      _intervalGetterFactory(std::make_shared<IntervalGetterFactory>(
          std::bind(&SoloHarmonizerVst::_onCrotchetsPerSecondAvailable, this,
                    _1),
          std::bind(&SoloHarmonizerVst::_onPlayheadCommand, this, _1))),
      _soloHarmonizer(
          std::make_unique<SoloHarmonizer>(_intervalGetterFactory, *this)),
      _playheadFactory(std::move(factory)) {}

const juce::String SoloHarmonizerVst::getName() const {
  return JucePlugin_Name;
}

void SoloHarmonizerVst::prepareToPlay(double sampleRate, int samplesPerBlock) {
  _samplesPerSecond = static_cast<int>(sampleRate);
  _intervalGetterFactory->setSampleRate(*_samplesPerSecond);
  _soloHarmonizer->prepareToPlay(*_samplesPerSecond, samplesPerBlock);
  _createPlayheadIfReady();
}

void SoloHarmonizerVst::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  _soloHarmonizer->releaseResources();
}

bool SoloHarmonizerVst::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono())
    return false;

  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;

  return true;
}

void SoloHarmonizerVst::processBlock(juce::AudioBuffer<float> &buffer,
                                     juce::MidiBuffer &) {
  _soloHarmonizer->processBlock(buffer.getWritePointer(0),
                                buffer.getNumSamples());
  incrementSampleCount(buffer.getNumSamples());
}

std::optional<float> SoloHarmonizerVst::getTimeInCrotchets() const {
  if (_playhead) {
    return _playhead->getTimeInCrotchets();
  } else {
    return std::nullopt;
  }
}

void SoloHarmonizerVst::incrementSampleCount(int increment) {
  if (_playhead) {
    _playhead->incrementSampleCount(increment);
  }
}

juce::AudioPlayHead *SoloHarmonizerVst::getJuceAudioPlayHead() const {
  return getPlayHead();
}

juce::AudioProcessorEditor *SoloHarmonizerVst::createEditor() {
  return new SoloHarmonizerEditor(*this, *_intervalGetterFactory);
}

void SoloHarmonizerVst::getStateInformation(juce::MemoryBlock &destData) {
  const auto &vectorView = _soloHarmonizer->getState();
  destData.append(vectorView.data(), vectorView.size());
}

void SoloHarmonizerVst::setStateInformation(const void *data, int sizeInBytes) {
  const auto uint8Data = static_cast<const uint8_t *>(data);
  const std::vector<uint8_t> vectorView{uint8Data, uint8Data + sizeInBytes};
  _soloHarmonizer->setState(vectorView);
}

void SoloHarmonizerVst::_createPlayheadIfReady() {
  if (_samplesPerSecond.has_value() && _crotchetsPerSecond.has_value()) {
    const auto mustSetPpqPosition =
        wrapperType ==
        juce::AudioProcessor::WrapperType::wrapperType_Standalone;
    const auto crotchetsPerSample =
        utils::getCrotchetsPerSample(*_crotchetsPerSecond, *_samplesPerSecond);
    _playhead = _playheadFactory(mustSetPpqPosition, *this, crotchetsPerSample);
  }
}

void SoloHarmonizerVst::_onCrotchetsPerSecondAvailable(
    float crotchetsPerSecond) {
  _crotchetsPerSecond = crotchetsPerSecond;
  _createPlayheadIfReady();
}

bool SoloHarmonizerVst::_onPlayheadCommand(PlayheadCommand command) {
  return _playhead ? _playhead->execute(command) : false;
}
} // namespace saint

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  using namespace saint;
  PlayheadFactory factory{
      [](bool mustSetPpqPosition,
         const JuceAudioPlayHeadProvider &playheadProvider,
         float crotchetsPerSample) -> std::unique_ptr<Playhead> {
        if (mustSetPpqPosition) {
          return std::make_unique<ProcessCallbackDrivenPlayhead>(
              crotchetsPerSample);
        } else {
          return std::make_unique<HostDrivenPlayhead>(playheadProvider);
        }
      }};
  return new SoloHarmonizerVst(std::move(factory));
}
