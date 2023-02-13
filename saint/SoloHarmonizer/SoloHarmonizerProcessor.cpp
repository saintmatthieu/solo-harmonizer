#include "SoloHarmonizerProcessor.h"
#include "HarmoPitchGetter.h"
#include "HarmoPitchHelper.h"
#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerHelper.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "rubberband/RubberBandStretcher.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
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

std::optional<juce::MidiFile> getMidiFile(const juce::String &filename) {
  juce::MidiFile midiFile;
  const juce::File file(filename);
  const auto stream = file.createInputStream();
  if (!midiFile.readFrom(*stream)) {
    return std::nullopt;
  }
  return midiFile;
}

std::vector<MidiNoteMsg>
getMidiNoteMessages(const juce::MidiMessageSequence &seq,
                    int ticksPerCrotchet) {
  const auto ticksPer32nd = ticksPerCrotchet / 8;
  std::vector<MidiNoteMsg> msgs;
  MidiNoteMsg *lastMsg = nullptr;
  for (auto it = seq.begin(); it != seq.end(); ++it) {
    const auto msg = (*it)->message;
    if (!msg.isNoteOnOrOff()) {
      continue;
    }
    const auto tick =
        static_cast<int>(msg.getTimeStamp() / ticksPer32nd + 0.5) *
        ticksPer32nd;
    if (msg.isNoteOn() && lastMsg && !lastMsg->isNoteOn &&
        lastMsg->tick == tick) {
      // This new NoteOn coincides with the previous NoteOff => let's delete
      // that NoteOff
      msgs.pop_back();
    }
    msgs.push_back({tick, msg.isNoteOn(), msg.getNoteNumber()});
    lastMsg = &msgs.back();
  }
  return msgs;
}
} // namespace

SoloHarmonizerProcessor::SoloHarmonizerProcessor(
    std::optional<RubberBand::RubberBandStretcher::Options> opts)
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::mono(), true)
              .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      _rbStretcherOptions(std::move(opts)),
      _loggerName(std::string{"SoloHarmonizerProcessor_"} +
                  std::to_string(instanceCounter++)),
      _logger(spdlog::basic_logger_mt(
          _loggerName, saint::generateLogFilename(_loggerName).string())) {
  _logger->set_level(saint::getLogLevelFromEnv());
  _logger->info("ctor {0}", _loggerName);
}

SoloHarmonizerProcessor::~SoloHarmonizerProcessor() {
  _logger->info("dtor {0}", _loggerName);
}

void SoloHarmonizerProcessor::setSemitoneShift(float value) {
  _pitchShifter->setSemitoneShift(value);
}

void SoloHarmonizerProcessor::setCustomPlayhead(
    std::weak_ptr<juce::AudioPlayHead> ph) {
  _logger->info("setting custom playhead");
  _customPlayhead = std::move(ph);
}

const juce::String SoloHarmonizerProcessor::getName() const {
  return JucePlugin_Name;
}

void SoloHarmonizerProcessor::prepareToPlay(double sampleRate,
                                            int samplesPerBlock) {
  _pitchShifter = std::make_unique<PitchShifter>(1, sampleRate, samplesPerBlock,
                                                 _rbStretcherOptions);
  constexpr auto pyinCppDefaultBlockSize = 2048;
  const auto pyinCppStepSize =
      std::max(samplesPerBlock, pyinCppDefaultBlockSize);
  _logger->info("prepareToPlay sampleRate={0} samplesPerBlock={1} "
                "pyinCppStepSize={2} _harmoPitchGetter={3} _pitchEstimate={4}",
                sampleRate, samplesPerBlock, pyinCppStepSize,
                _harmoPitchGetter != nullptr, _pitchEstimate != std::nullopt);
  _pitchEstimator = std::make_unique<PyinCpp>(
      sampleRate, pyinCppDefaultBlockSize, pyinCppStepSize);
  _pitchEstimator->reserve(samplesPerBlock); // I guess ...
}

void SoloHarmonizerProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  _pitchEstimator.reset();
  _pitchShifter.reset();
  _pitchEstimate.reset();
  _logger->info("releaseResources");
  _logger->flush();
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
  _logger->trace("processBlock");
  juce::ignoreUnused(midiMessages);
  if (!_harmoPitchGetter) {
    return;
  }
  const auto readPtr = buffer.getReadPointer(0);
  _updatePitchEstimate(readPtr, (size_t)buffer.getNumSamples());
  _runPitchShift(buffer);
}

juce::AudioProcessorEditor *SoloHarmonizerProcessor::createEditor() {
  const auto editor = new SoloHarmonizerEditor(*this, *this);
  return editor;
}

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

namespace {
std::string getTrackName(
    const juce::MidiMessageSequence &track,
    const std::function<bool(const juce::MidiMessage &)> &pred,
    const std::function<std::string(const juce::MidiMessage &)> &nameGetter) {
  const auto pHolder =
      std::find_if(track.begin(), track.end(), [&pred](auto it) {
        const juce::MidiMessage &msg = it->message;
        return pred(msg);
      });
  if (pHolder == track.end()) {
    return "";
  }
  const juce::MidiMessage &msg = (*pHolder)->message;
  return nameGetter(msg);
}
} // namespace

std::vector<TrackInfo>
SoloHarmonizerProcessor::onMidiFileChosen(const std::filesystem::path &path) {
  _logger->info("loadMidiFile path={0}", path.string());
  _midiFile = getMidiFile(path.string());
  _reloadIfReady();
  if (!_midiFile) {
    return {};
  }
  std::vector<TrackInfo> tracks;
  for (auto i = 0; i < _midiFile->getNumTracks(); ++i) {
    const auto track = _midiFile->getTrack(i);
    if (std::all_of(track->begin(), track->end(), [](auto it) {
          const auto &msg = it->message;
          return !msg.isNoteOnOrOff();
        })) {
      continue;
    }
    auto trackName = getTrackName(
        *track,
        [](const juce::MidiMessage &msg) { return msg.isTrackNameEvent(); },
        [](const juce::MidiMessage &msg) {
          return msg.getTextFromTextMetaEvent().toStdString();
        });
    if (trackName.empty()) {
      trackName = getTrackName(
          *track,
          [](const juce::MidiMessage &msg) { return msg.isProgramChange(); },
          [](const juce::MidiMessage &msg) {
            return std::string{juce::MidiMessage::getGMInstrumentName(
                msg.getProgramChangeNumber())};
          });
    }
    tracks.push_back({trackName});
  }
  return tracks;
}

void SoloHarmonizerProcessor::onTrackSelected(TrackType trackType,
                                              int newTrackNumber) {
  auto &trackNumber =
      trackType == TrackType::played ? _playedTrackNumber : _harmonyTrackNumber;
  const auto reload = newTrackNumber != trackNumber;
  trackNumber = newTrackNumber;
  if (reload) {
    _reloadIfReady();
  }
}

void SoloHarmonizerProcessor::_reloadIfReady() {
  if (!_midiFile.has_value() || !_playedTrackNumber.has_value() ||
      !_harmonyTrackNumber.has_value()) {
    return;
  }
  const auto ticksPerCrotchet = static_cast<int>(_midiFile->getTimeFormat());
  if (ticksPerCrotchet <= 0) {
    _logger->error("MIDI file uses (yet) unsupported SMPTE format");
    _midiFile.reset();
    return;
  }

  const auto playedSeq = getMidiNoteMessages(
      *_midiFile->getTrack(*_playedTrackNumber), ticksPerCrotchet);
  const auto harmoSeq = getMidiNoteMessages(
      *_midiFile->getTrack(*_harmonyTrackNumber), ticksPerCrotchet);
  const auto input = toHarmoNoteSpans(playedSeq, harmoSeq);
  if (input.empty()) {
    _logger->warn("toHarmoPitchGetterInput returned empty vector");
    return;
  } else {
    if (!ticksPerCrotchet) {
      _logger->warn("ticksPerCrotchet nullptr");
    } else {
      _logger->info("ticksPerCrotchet {0}", ticksPerCrotchet);
    }
    _logger->info("toHarmoPitchGetterInput call successful");
    _harmoPitchGetter = std::make_unique<saint::HarmoPitchGetter>(input);
  }
}

void SoloHarmonizerProcessor::_updatePitchEstimate(float const *p, size_t s) {
  const std::vector<float> v{p, p + s};
  const auto pitches = _pitchEstimator->feed(v);
  if (pitches.empty() || pitches[0] < 20 || pitches[0] > 4000) {
    if (_pitchEstimate) {
      _logger->debug("_pitchEstimate == nullopt");
    }
    _pitchEstimate.reset();
  } else {
    if (!_pitchEstimate) {
      _logger->debug("_pitchEstimate != nullopt");
    }
    // Still don't know why there may be more than one pitches.
    _pitchEstimate.emplace(pitches[0]);
  }
}

void SoloHarmonizerProcessor::_runPitchShift(juce::AudioBuffer<float> &buffer) {
  const auto pitchShift = _getHarmonySemitones();
  _logger->debug("_getHarmonySemitones() returned {0}",
                 pitchShift ? std::to_string(*pitchShift) : "nullopt");
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
  if (!_getPositionLogged) {
    _logger->debug("1st getPosition call was {0}",
                   position ? "successful" : "not successful");
    _getPositionLogged = true;
  }
  if (!position) {
    return std::nullopt;
  }
  const auto ppq = position->getPpqPosition();
  if (!ppq) {
    if (!_getPpqPositionLogged) {
      _logger->warn("could not get PPQ position ??");
      _getPpqPositionLogged = true;
    }
    return std::nullopt;
  }
  // Don't know why it's a double and not an int ...
  const auto tick = static_cast<int>(*ppq + 0.5);
  return _harmoPitchGetter->getHarmoInterval(tick, *_pitchEstimate);
}

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new SoloHarmonizerProcessor(std::nullopt);
}
} // namespace saint
