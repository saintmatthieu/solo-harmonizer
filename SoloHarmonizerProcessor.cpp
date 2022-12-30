#include "SoloHarmonizerProcessor.h"
#include "SoloHarmonizerEditor.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "rubberband/RubberBandStretcher.h"
#include <algorithm>
#include <memory>
#include <optional>

namespace {
juce::MidiFile getMidiFile(const juce::String &filename) {
  juce::MidiFile midiFile;
  const juce::File file(filename);
  const auto stream = file.createInputStream();
  if (!midiFile.readFrom(*stream)) {
    jassertfalse;
  }
  return midiFile;
}
} // namespace

//==============================================================================
SoloHarmonizerProcessor::SoloHarmonizerProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::mono(), true)
              .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      _fileFilter("*.mid;*.midi", "", ""),
      _fileBrowserComponent(
          juce::FileBrowserComponent::FileChooserFlags::openMode |
              juce::FileBrowserComponent::FileChooserFlags::canSelectFiles,
          juce::File(), &_fileFilter, nullptr),
      _pitchDisplay("Sample diff") {
  _fileBrowserComponent.addListener(this);
  _fileBrowserComponent.setSize(250, 250);
  _pitchDisplay.setSize(250, 100);
  const juce::File file("C:/Users/saint/Downloads/test.xml");
  const auto xml = juce::XmlDocument::parse(file);
  const auto masterMidi = getMidiFile(xml->getAttributeValue(0));
  const auto slaveMidi = getMidiFile(xml->getAttributeValue(1));
  const juce::MidiMessageSequence *masterSequence = masterMidi.getTrack(1);
  std::vector<double> timestamps;
  std::vector<std::optional<int>> noteNumbers;
  std::optional<double> currentNoteoffTimestamp;
  for (auto it = masterSequence->begin(); it != masterSequence->end(); ++it) {
    const auto message = (*it)->message;
    if (!message.isNoteOn()) {
      continue;
    }
    const auto timestamp = message.getTimeStamp();
    if (currentNoteoffTimestamp && timestamp > currentNoteoffTimestamp) {
      timestamps.push_back(*currentNoteoffTimestamp);
      noteNumbers.push_back(std::nullopt);
      currentNoteoffTimestamp = std::nullopt;
    }
    timestamps.push_back(message.getTimeStamp());
    noteNumbers.push_back(message.getNoteNumber());
  }
  juce::MessageManager::getInstance()->registerBroadcastListener(this);
}

SoloHarmonizerProcessor::~SoloHarmonizerProcessor() {}

void SoloHarmonizerProcessor::actionListenerCallback(
    const juce::String &message) {
  _pitchDisplay.setText(message, juce::NotificationType::dontSendNotification);
}

//==============================================================================
const juce::String SoloHarmonizerProcessor::getName() const {
  return JucePlugin_Name;
}

bool SoloHarmonizerProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool SoloHarmonizerProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool SoloHarmonizerProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double SoloHarmonizerProcessor::getTailLengthSeconds() const { return 0.0; }

int SoloHarmonizerProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int SoloHarmonizerProcessor::getCurrentProgram() { return 0; }

void SoloHarmonizerProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String SoloHarmonizerProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void SoloHarmonizerProcessor::changeProgramName(int index,
                                                const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void SoloHarmonizerProcessor::prepareToPlay(double sampleRate,
                                            int samplesPerBlock) {
  _stretcher = std::make_unique<RubberBand::RubberBandStretcher>(
      sampleRate, 1,
      RubberBand::RubberBandStretcher::OptionProcessRealTime |
          RubberBand::RubberBandStretcher::OptionTransientsCrisp |
          RubberBand::RubberBandStretcher::OptionDetectorCompound |
          RubberBand::RubberBandStretcher::OptionPitchHighConsistency |
          RubberBand::RubberBandStretcher::OptionChannelsTogether |
          RubberBand::RubberBandStretcher::OptionEngineFaster);
  jassert(_stretcher->getEngineVersion() ==
          2 /* version 2, matching OptionEngineFaster */);
  const auto uSamplesPerBlock = (size_t)samplesPerBlock;
  const auto toPad = (size_t)_stretcher->getPreferredStartPad();
  _dummyBuffer = std::make_unique<std::vector<float>>(
      std::max(uSamplesPerBlock, toPad), 0.f);
  _stretcher->setMaxProcessSize(uSamplesPerBlock);
  _stretcher->setPitchScale(1.0);
  if (toPad > 0u) {
    auto data = _dummyBuffer->data();
    _stretcher->process(&data, toPad, false);
  }
  _numLeadingSamplesToDrop = _stretcher->getStartDelay();
  _pyinCpp =
      std::make_unique<PyinCpp>(static_cast<int>(sampleRate), samplesPerBlock);
}

void SoloHarmonizerProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool SoloHarmonizerProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void SoloHarmonizerProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                           juce::MidiBuffer &midiMessages) {
  juce::ignoreUnused(midiMessages);
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  jassert(totalNumInputChannels == 1);
  jassert(totalNumOutputChannels == 1);
  const auto numSamples = (size_t)buffer.getNumSamples();

  _stretcher->process(buffer.getArrayOfReadPointers(), numSamples, false);
  auto numAvailableSamples = _stretcher->available();

  auto data = _dummyBuffer->data();
  while (numAvailableSamples > 0 && _numLeadingSamplesToDrop > 0) {
    const auto toDropInNextCall =
        std::min((size_t)numAvailableSamples, _numLeadingSamplesToDrop);
    _stretcher->retrieve(&data, toDropInNextCall);
    _numLeadingSamplesToDrop -= toDropInNextCall;
    numAvailableSamples -= toDropInNextCall;
  }

  const auto pData = buffer.getArrayOfWritePointers();
  const auto numLeadingSamplesToSilence =
      (size_t)std::max((int)numSamples - numAvailableSamples, 0);
  memset(pData[0], 0.f, numLeadingSamplesToSilence);
  auto repaint = false;
  if (numAvailableSamples > 0) {
    const auto offsetPointer = pData[0] + numLeadingSamplesToSilence;
    const auto numSamplesToRetrieve = numSamples - numLeadingSamplesToSilence;
    const auto newNumAvailableSamplesMin =
        _numAvailableSamplesMin == std::nullopt
            ? numSamplesToRetrieve
            : std::min(numSamplesToRetrieve, *_numAvailableSamplesMin);
    if (newNumAvailableSamplesMin != _numAvailableSamplesMin) {
      repaint = true;
      _numAvailableSamplesMin = newNumAvailableSamplesMin;
    }
    _stretcher->retrieve(&offsetPointer, numSamplesToRetrieve);
    numAvailableSamples -= numSamplesToRetrieve;
  }
  if (_numAvailableSamplesMin != std::nullopt &&
      numAvailableSamples > _numRemainingSamplesMax) {
    repaint = true;
    _numRemainingSamplesMax = numAvailableSamples;
  }
  if (repaint) {
    juce::MessageManager::getInstance()->deliverBroadcastMessage(
        std::to_string(*_numAvailableSamplesMin) + ", " +
        std::to_string(*_numRemainingSamplesMax));
  }
  // std::vector<float> vector(numSamples);
  // const auto channel = buffer.getReadPointer(0);
  // for (auto i = 0u; i < numSamples; ++i) {
  //   vector[i] = channel[i];
  // }
  // _pyinCpp->feed(vector);
  // const auto pitch = _pyinCpp->getPitches();
}

//==============================================================================
bool SoloHarmonizerProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *SoloHarmonizerProcessor::createEditor() {
  const auto editor = new SoloHarmonizerEditor(*this);
  editor->addAndMakeVisible(_pitchDisplay);
  return editor;
}

//==============================================================================
void SoloHarmonizerProcessor::getStateInformation(juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::ignoreUnused(destData);
}

void SoloHarmonizerProcessor::setStateInformation(const void *data,
                                                  int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  juce::ignoreUnused(data, sizeInBytes);
}

void SoloHarmonizerProcessor::fileDoubleClicked(const juce::File &file) {
  juce::MidiFile midiFile;
  const auto stream = file.createInputStream();
  if (!midiFile.readFrom(*stream)) {
    jassertfalse;
  }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new SoloHarmonizerProcessor();
}
