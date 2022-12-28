#include "SoloHarmonizerProcessor.h"
#include "SoloHarmonizerEditor.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "rubberband/RubberBandStretcher.h"
#include <memory>

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
      _pitchDisplay("Detected Pitch") {
  _fileBrowserComponent.addListener(this);
  _fileBrowserComponent.setSize(250, 250);
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
}

SoloHarmonizerProcessor::~SoloHarmonizerProcessor() {}

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
  _stretcher = std::make_unique<RubberBand::RubberBandStretcher>(sampleRate, 1);
  _stretcher->setMaxProcessSize(static_cast<size_t>(samplesPerBlock));
  _stretcher->setPitchScale(2.0);
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
  const auto numSamples = static_cast<size_t>(buffer.getNumSamples());
  _stretcher->process(buffer.getArrayOfReadPointers(), numSamples, false);
  if (_stretcher->available() >= buffer.getNumSamples()) {
    _stretcher->retrieve(buffer.getArrayOfWritePointers(), numSamples);
  } else {
    // stuff
  }
}

//==============================================================================
bool SoloHarmonizerProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *SoloHarmonizerProcessor::createEditor() {
  const auto editor = new SoloHarmonizerEditor(*this);
  editor->addAndMakeVisible(_fileBrowserComponent);
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
