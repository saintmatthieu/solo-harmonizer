#include "SoloHarmonizer.h"
#include "HarmoPitchGetter.h"
#include "HarmoPitchHelper.h"
#include "SoloHarmonizerEditor.h"
#include "SoloHarmonizerHelper.h"
#include "Tickers/BuiltinTicker.h"
#include "Tickers/HostTicker.h"
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

SoloHarmonizer::SoloHarmonizer(
    std::optional<RubberBand::RubberBandStretcher::Options> opts)
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::mono(), true)
              .withOutput("Output", juce::AudioChannelSet::mono(), true)),
      _rbStretcherOptions(std::move(opts)),
      _loggerName(std::string{"SoloHarmonizer_"} +
                  std::to_string(instanceCounter++)),
      _logger(spdlog::basic_logger_mt(
          _loggerName, saint::generateLogFilename(_loggerName).string())) {
  _logger->set_level(saint::getLogLevelFromEnv());
  _logger->info("ctor {0}", _loggerName);
}

SoloHarmonizer::~SoloHarmonizer() { _logger->info("dtor {0}", _loggerName); }

void SoloHarmonizer::setSemitoneShift(float value) {
  _pitchShifter->setSemitoneShift(value);
}

const juce::String SoloHarmonizer::getName() const { return JucePlugin_Name; }

void SoloHarmonizer::prepareToPlay(double sampleRate, int samplesPerBlock) {
  _audioConfig.samplesPerBlock = samplesPerBlock;
  _audioConfig.samplesPerSecond = sampleRate;
  _pitchShifter = std::make_unique<PitchShifter>(1, sampleRate, samplesPerBlock,
                                                 _rbStretcherOptions);
  _logger->info(
      "prepareToPlay sampleRate={0} samplesPerBlock={1} _harmoPitchGetter={2}",
      sampleRate, samplesPerBlock, _harmoPitchGetter != nullptr);
  _ticker.reset(_useHostPlayhead
                    ? static_cast<ITicker *>(
                          new HostTicker([this]() { return getPlayHead(); }))
                    : static_cast<ITicker *>(new BuiltinTicker(_audioConfig)));
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
  _logger->trace("processBlock");
  juce::ignoreUnused(midiMessages);
  if (!_harmoPitchGetter) {
    return;
  }
  _runPitchShift(buffer);
  // must be called after processing
  // TODO: how to remove this trap ?
  _ticker->incrementSampleCount(buffer.getNumSamples());
}

juce::AudioProcessorEditor *SoloHarmonizer::createEditor() {
  const auto editor = new SoloHarmonizerEditor(*this, *this);
  return editor;
}

void SoloHarmonizer::getStateInformation(juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::ignoreUnused(destData);
}

void SoloHarmonizer::setStateInformation(const void *data, int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  juce::ignoreUnused(data, sizeInBytes);
}

namespace {
template <typename T>
std::optional<T>
getFirstFromTrack(const juce::MidiMessageSequence &track,
                  const std::function<bool(const juce::MidiMessage &)> &pred,
                  const std::function<T(const juce::MidiMessage &)> &getter) {
  const auto pHolder =
      std::find_if(track.begin(), track.end(), [&pred](auto it) {
        const juce::MidiMessage &msg = it->message;
        return pred(msg);
      });
  if (pHolder == track.end()) {
    return std::nullopt;
  }
  const juce::MidiMessage &msg = (*pHolder)->message;
  return getter(msg);
}

int getCrotchetsPerSecond(const juce::MidiFile &midiFile) {
  // This makes the assumption that the tempo stays constant.
  // For tempo-changing pieces, something more elaborate would be needed.
  // But again, this is only for the built-in tick provider ; a proper host
  // would do that.
  const auto secondsPerCrotchet = getFirstFromTrack<double>(
      *midiFile.getTrack(0),
      [](const juce::MidiMessage &msg) { return msg.isTempoMetaEvent(); },
      [](const juce::MidiMessage &msg) {
        return msg.getTempoSecondsPerQuarterNote();
      });
  return secondsPerCrotchet.has_value()
             ? static_cast<int>(1 / (*secondsPerCrotchet) + 0.5)
             : 0;
}
} // namespace

std::vector<TrackInfo>
SoloHarmonizer::onMidiFileChosen(const std::filesystem::path &path) {
  _logger->info("loadMidiFile path={0}", path.string());
  _midiFile = getMidiFile(path.string());
  _reloadIfReady();
  if (!_midiFile) {
    return {};
  }
  _audioConfig.ticksPerCrotchet = _midiFile->getTimeFormat();
  _audioConfig.crotchetsPerSecond = getCrotchetsPerSecond(*_midiFile);
  std::vector<TrackInfo> tracks;
  for (auto i = 0; i < _midiFile->getNumTracks(); ++i) {
    const auto track = _midiFile->getTrack(i);
    if (std::all_of(track->begin(), track->end(), [](auto it) {
          const auto &msg = it->message;
          return !msg.isNoteOnOrOff();
        })) {
      continue;
    }
    auto trackName = getFirstFromTrack<std::string>(
        *track,
        [](const juce::MidiMessage &msg) { return msg.isTrackNameEvent(); },
        [](const juce::MidiMessage &msg) {
          return msg.getTextFromTextMetaEvent().toStdString();
        });
    if (!trackName.has_value()) {
      trackName = getFirstFromTrack<std::string>(
          *track,
          [](const juce::MidiMessage &msg) { return msg.isProgramChange(); },
          [](const juce::MidiMessage &msg) {
            return juce::MidiMessage::getGMInstrumentName(
                msg.getProgramChangeNumber());
          });
    }
    tracks.push_back({trackName.has_value() ? *trackName : ""});
  }
  return tracks;
}

void SoloHarmonizer::onTrackSelected(TrackType trackType, int newTrackNumber) {
  auto &trackNumber =
      trackType == TrackType::played ? _playedTrackNumber : _harmonyTrackNumber;
  const auto reload = newTrackNumber != trackNumber;
  trackNumber = newTrackNumber;
  if (reload) {
    _reloadIfReady();
  }
}

bool SoloHarmonizer::getUseHostPlayhead() const { return _useHostPlayhead; }

void SoloHarmonizer::setUseHostPlayhead(bool useHostPlayhead) {
  _useHostPlayhead = useHostPlayhead;
}

void SoloHarmonizer::_reloadIfReady() {
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

void SoloHarmonizer::_runPitchShift(juce::AudioBuffer<float> &buffer) {
  const auto tick = _ticker->getTick();
  if (!tick) {
    return;
  }
  const auto pitchShift = _harmoPitchGetter->getHarmoInterval(*tick);
  _logger->debug("_harmoPitchGetter->getHarmoInterval() returned {0}",
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
} // namespace saint

// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new saint::SoloHarmonizer(std::nullopt);
}
