#include "SoloHarmonizerProcessor.h"
#include "HarmoPitchFileReader.h"
#include "HarmoPitchGetter.h"
#include "SoloHarmonizerEditor.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "rubberband/RubberBandStretcher.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>

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
}

void SoloHarmonizerProcessor::setSemitoneShift(float value) {
  _pitchShifter->setSemitoneShift(value);
}

void SoloHarmonizerProcessor::setCustomPlayhead(
    std::weak_ptr<juce::AudioPlayHead> ph) {
  _customPlayhead = std::move(ph);
}

//==============================================================================
const juce::String SoloHarmonizerProcessor::getName() const {
  return JucePlugin_Name;
}

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
  _pitchEstimator.reset();
  _pitchShifter.reset();
}

bool SoloHarmonizerProcessor::isBusesLayoutSupported(
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

void SoloHarmonizerProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                           juce::MidiBuffer &midiMessages) {
  juce::ignoreUnused(midiMessages);
  if (!_harmoPitchGetter) {
    return;
  }
  const auto readPtr = buffer.getReadPointer(0);
  _updatePitchEstimate(readPtr, (size_t)buffer.getNumSamples());
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
  loadConfigFile(file.getFullPathName().toStdString());
}

void SoloHarmonizerProcessor::loadConfigFile(const std::filesystem::path &path,
                                             int *ticksPerCrotchet) {
  const auto input = saint::toHarmoPitchGetterInput(path, ticksPerCrotchet);
  _harmoPitchGetter = std::make_unique<saint::HarmoPitchGetter>(input);
}

void SoloHarmonizerProcessor::_updatePitchEstimate(float const *p, size_t s) {
  const std::vector<float> v{p, p + s};
  const auto pitches = _pitchEstimator->feed(v);
  if (pitches.empty() || pitches[0] < 20 || pitches[0] > 4000) {
    _pitchEstimate.reset();
  } else {
    // Still don't know why there may be more than one pitches.
    _pitchEstimate.emplace(pitches[0]);
  }
}

void SoloHarmonizerProcessor::_runPitchShift(juce::AudioBuffer<float> &buffer) {
  const auto pitchShift = _getHarmonySemitones();
  juce::dsp::AudioBlock<float> block{buffer};
  _pitchShifter->setMixPercentage(pitchShift ? 50 : 0);
  if (pitchShift) {
    _pitchShifter->setSemitoneShift(*pitchShift);
  }
  _pitchShifter->processBuffer(block);
  // TODO: check if these additional steps are necessary or not.
  const auto bp = block.getChannelPointer(0);
  auto ap = buffer.getWritePointer(0);
  memcpy(ap, bp, buffer.getNumSamples() * sizeof(float));
}

std::optional<float> SoloHarmonizerProcessor::_getHarmonySemitones() {
  const juce::AudioPlayHead *playhead =
      !_customPlayhead.expired() ? _customPlayhead.lock().get() : getPlayHead();
  if (!_harmoPitchGetter || !playhead || !_pitchEstimate) {
    return std::nullopt;
  }
  const auto position = playhead->getPosition();
  if (!position) {
    return std::nullopt;
  }
  const auto ppq = position->getPpqPosition();
  if (!ppq) {
    return std::nullopt;
  }
  // Don't know why it's a double and not an int ...
  const auto tick = static_cast<int>(*ppq + 0.5);
  return _harmoPitchGetter->getHarmoInterval(tick, *_pitchEstimate);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new SoloHarmonizerProcessor(std::nullopt);
}
