#include "SoloHarmonizerVst.h"
#include "Factory/IntervalGetterFactory.h"
#include "SoloHarmonizerEditor.h"

#include <cassert>

namespace saint {
SoloHarmonizerVst::SoloHarmonizerVst()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::mono(), true)
              .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      _intervalGetterFactory(std::make_shared<IntervalGetterFactory>()),
      _soloHarmonizer(
          std::make_unique<SoloHarmonizer>(_intervalGetterFactory, *this)) {}

const juce::String SoloHarmonizerVst::getName() const {
  return JucePlugin_Name;
}

void SoloHarmonizerVst::prepareToPlay(double sampleRate, int samplesPerBlock) {
  _soloHarmonizer->prepareToPlay(sampleRate, samplesPerBlock);
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
}

std::optional<double> SoloHarmonizerVst::getTimeInCrotchets() const {
  const juce::AudioPlayHead *playhead = getPlayHead();
  if (!playhead) {
    // TODO log
    return std::nullopt;
  }
  const auto position = playhead->getPosition();
  if (!position) {
    return std::nullopt;
  }
  const auto ppq = position->getPpqPosition();
  if (!ppq) {
    // TODO log
    return std::nullopt;
  }
  return *ppq;
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
} // namespace saint

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new saint::SoloHarmonizerVst();
}
