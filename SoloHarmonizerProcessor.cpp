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
      _pitchDisplay("Sample diff"), _sbsmsWrapper(std::vector<float>{}) {
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
  const auto blocksPerSecond = sampleRate / samplesPerBlock;
  const auto radPerSec = 2 * 3.1416f;
  _phaseDelta = (float)(radPerSec / blocksPerSecond);
  _pitchShifter =
      std::make_unique<PitchShifter>(1, sampleRate, samplesPerBlock);
  _pitchShifter->setFormantPreserving(true);
  _pitchShifter->setMixPercentage(100);
}

void SoloHarmonizerProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  _pitchShifter.reset();
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
  _phase += _phaseDelta;
  const auto newShift = std::sinf(_phase) * _semitoneModulationAmp;
  _pitchShifter->setSemitoneShift(7);
  juce::dsp::AudioBlock<float> block{buffer};
  _pitchShifter->processBuffer(block);
  const auto bp = block.getChannelPointer(0);
  auto ap = buffer.getWritePointer(0);
  memcpy(ap, bp, buffer.getNumSamples() * sizeof(float));
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
