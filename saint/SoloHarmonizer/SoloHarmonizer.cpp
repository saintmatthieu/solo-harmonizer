#include "SoloHarmonizer.h"
#include "Factory/HarmoPitchGetterFactory.h"
#include "HarmoPitchTypes.h"
#include "Playheads/BuiltinPlayhead.h"
#include "Playheads/HostPlayhead.h"
#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerHelper.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "rubberband/RubberBandStretcher.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "SoloHarmonizerEditor.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>

namespace saint {
namespace {
static std::atomic<int> instanceCounter = 0;
} // namespace

SoloHarmonizer::SoloHarmonizer(
    std::optional<RubberBand::RubberBandStretcher::Options> opts,
    std::shared_ptr<EditorsFactoryView> editorsFactoryView,
    std::shared_ptr<ProcessorsFactoryView> processorsFactoryView)
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::mono(), true)
              .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      _rbStretcherOptions(std::move(opts)),
      _editorsFactoryView(std::move(editorsFactoryView)),
      _processorsFactoryView(std::move(processorsFactoryView)),
      _loggerName(std::string{"SoloHarmonizer_"} +
                  std::to_string(instanceCounter++)),
      _logger(spdlog::basic_logger_mt(
          _loggerName, saint::generateLogFilename(_loggerName).string())),
      _playhead(
          std::make_unique<HostPlayhead>([this]() { return getPlayHead(); })) {
  _logger->set_level(saint::getLogLevelFromEnv());
  _logger->info("ctor {0}", _loggerName);
}

SoloHarmonizer::~SoloHarmonizer() { _logger->info("dtor {0}", _loggerName); }

void SoloHarmonizer::setSemitoneShift(float value) {
  _pitchShifter->setSemitoneShift(value);
}

const juce::String SoloHarmonizer::getName() const { return JucePlugin_Name; }

void SoloHarmonizer::prepareToPlay(double sampleRate, int samplesPerBlock) {
  _pitchShifter = std::make_unique<DavidCNAntonia::PitchShifter>(
      1, sampleRate, samplesPerBlock, _rbStretcherOptions);
  _logger->info("prepareToPlay sampleRate={0} samplesPerBlock={1}", sampleRate,
                samplesPerBlock);
}

void SoloHarmonizer::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  _pitchShifter.reset();
  _logger->info("releaseResources");
  _logger->flush();
}

bool SoloHarmonizer::isBusesLayoutSupported(const BusesLayout &layouts) const {
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

void SoloHarmonizer::processBlock(juce::AudioBuffer<float> &buffer,
                                  juce::MidiBuffer &midiMessages) {
  juce::ignoreUnused(midiMessages);
  _logger->trace("processBlock");
  if (!_processorsFactoryView->hasHarmoPitchGetter()) {
    return;
  }
  const auto harmoPitchGetter = _processorsFactoryView->getHarmoPitchGetter();
  if (!harmoPitchGetter) {
    return;
  }
  const auto tick = _playhead->getTimeInCrotchets();
  if (!tick) {
    // TODO logging
    return;
  }
  const auto pitchShift = harmoPitchGetter->getHarmoInterval(*tick);
  _logger->debug("_harmoPitchGetter->getHarmoInterval() returned {0}",
                 pitchShift ? std::to_string(*pitchShift) : "nullopt");
  juce::dsp::AudioBlock<float> block{buffer};
  _pitchShifter->setMixPercentage(pitchShift ? 50.f : 0.f);
  if (pitchShift) {
    _pitchShifter->setSemitoneShift(*pitchShift);
  }
  _pitchShifter->processBuffer(block);
  // TODO: check if these additional steps are necessary or not.
  const auto bp = block.getChannelPointer(0);
  auto ap = buffer.getWritePointer(0);
  memcpy(ap, bp, buffer.getNumSamples() * sizeof(float));
  // must be called after processing
  // TODO: how to remove this trap ?
  _playhead->incrementSampleCount(buffer.getNumSamples());
}

juce::AudioProcessorEditor *SoloHarmonizer::createEditor() {
  return new SoloHarmonizerEditor(*this, *_editorsFactoryView);
}

void SoloHarmonizer::getStateInformation(juce::MemoryBlock &destData) {
  const auto &vectorView = _processorsFactoryView->getState();
  destData.append(vectorView.data(), vectorView.size());
}

void SoloHarmonizer::setStateInformation(const void *data, int sizeInBytes) {
  const auto uint8Data = static_cast<const uint8_t *>(data);
  const std::vector<uint8_t> vectorView{uint8Data, uint8Data + sizeInBytes};
  _processorsFactoryView->setState(vectorView);
}
} // namespace saint

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  const auto harmoPitchGetterFactory =
      std::make_shared<saint::HarmoPitchGetterFactory>();
  return new saint::SoloHarmonizer(std::nullopt, harmoPitchGetterFactory,
                                   harmoPitchGetterFactory);
}
