#include "SoloHarmonizerProcessor.h"
#include "SoloHarmonizerEditor.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "libpyincpp.h"
#include "rubberband/RubberBandStretcher.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
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

int getCentsFromA4(float hz) {
  constexpr auto A4 = 440.f;
  return static_cast<int>(1200 * log2f(hz / A4) + .5f);
}

float getSemitoneShiftForEMinorThirdHarmonization(float hz) {
  // Beginning from A
  constexpr std::array<int, 12> bMinorNaturalMap{
      0, // A
      0, // A#
      0, // B
      0, // C
      0, // C#
      0, // D
      0, // D#
      0, // E
      0, // F
      0, // F#
      0, // G
      0, // G#
  };
  constexpr std::array<std::optional<int>, 12> hotelCaliforniaMap{
      std::nullopt, // A
      std::nullopt, // A#
      3,            // B
      std::nullopt, // C
      std::nullopt, // C#
      4,            // D
      std::nullopt, // D#
      std::nullopt, // E
      std::nullopt, // F
      5,            // F#
      std::nullopt, // G
      std::nullopt, // G#
  };
}
} // namespace

//==============================================================================
SoloHarmonizerProcessor::SoloHarmonizerProcessor(
    std::optional<RubberBand::RubberBandStretcher::Options> opts)
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::mono(), true)
              .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      _rbStretcherOptions(std::move(opts)), _fileFilter("*.mid;*.midi", "", ""),
      _fileBrowserComponent(
          juce::FileBrowserComponent::FileChooserFlags::openMode |
              juce::FileBrowserComponent::FileChooserFlags::canSelectFiles,
          juce::File(), &_fileFilter, nullptr),
      _pitchDisplay("Sample diff") {
  juce::MessageManager::getInstance();
  _fileBrowserComponent.addListener(this);
  _fileBrowserComponent.setSize(250, 250);
  _pitchDisplay.setSize(250, 100);

  constexpr auto pitchLogName = "C:/Users/saint/Downloads/pitchLog.txt";
  if (std::filesystem::exists(pitchLogName)) {
    std::filesystem::remove(pitchLogName);
  }
  _pitchLog = std::ofstream{pitchLogName};
}

SoloHarmonizerProcessor::~SoloHarmonizerProcessor() {}

void SoloHarmonizerProcessor::setSemitoneShift(float value) {
  _pitchShifter->setSemitoneShift(value);
}

//==============================================================================
const juce::String SoloHarmonizerProcessor::getName() const {
  return JucePlugin_Name;
}

bool SoloHarmonizerProcessor::acceptsMidi() const { return false; }

bool SoloHarmonizerProcessor::producesMidi() const { return false; }

bool SoloHarmonizerProcessor::isMidiEffect() const { return false; }

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
  _pitchShifter = std::make_unique<PitchShifter>(1, sampleRate, samplesPerBlock,
                                                 _rbStretcherOptions);
  _pitchShifter->setFormantPreserving(true);
  _pitchShifter->setMixPercentage(100);

  // TODO: check that `samplesPerBlock` is appropriate for
  // `PyinCpp::_DEFAULT_BLOCK_SIZE` (2048) and `PyinCpp::_DEFAULT_STEP_SIZE`
  // (512).
  constexpr auto pyinCppDefaultBlockSize = 2048;
  assert(pyinCppDefaultBlockSize % samplesPerBlock == 0);
  _pitchEstimator = std::make_unique<PyinCpp>(
      sampleRate, pyinCppDefaultBlockSize, samplesPerBlock);
  _pitchEstimator->reserve(samplesPerBlock); // I guess ...
}

void SoloHarmonizerProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  _pitchShifter.reset();
}

bool SoloHarmonizerProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;

  return true;
}

void SoloHarmonizerProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                           juce::MidiBuffer &midiMessages) {
  juce::ignoreUnused(midiMessages);
  const auto readPtr = buffer.getReadPointer(0);
  _runPitchEstimate(readPtr, (size_t)buffer.getNumSamples());
  _runPitchShift(buffer);
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

void SoloHarmonizerProcessor::_runPitchEstimate(float const *p, size_t s) {
  const std::vector<float> v{p, p + s};
  const auto pitches = _pitchEstimator->feed(v);
  auto separator = ", ";
  for (const auto &pitch : pitches) {
    _pitchLog << separator << pitch;
  }
  _pitchLog << std::endl;
}

void SoloHarmonizerProcessor::_runPitchShift(juce::AudioBuffer<float> &buffer) {
  juce::dsp::AudioBlock<float> block{buffer};
  _pitchShifter->processBuffer(block);
  // TODO: check if these additional steps are necessary or not.
  const auto bp = block.getChannelPointer(0);
  auto ap = buffer.getWritePointer(0);
  memcpy(ap, bp, buffer.getNumSamples() * sizeof(float));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new SoloHarmonizerProcessor(std::nullopt);
}
