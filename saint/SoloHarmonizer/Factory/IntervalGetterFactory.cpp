#include "IntervalGetterFactory.h"
#include "IntervalGetter.h"
#include "IntervalGetterDebugCb.h"
#include "IntervalHelper.h"
#include "IntervalTypes.h"
#include "Utils.h"

namespace saint {

namespace {
std::optional<juce::MidiFile> getJuceMidiFile(const juce::String &filename) {
  juce::MidiFile midiFile;
  const juce::File file(filename);
  const auto stream = file.createInputStream();
  if (!midiFile.readFrom(*stream)) {
    return std::nullopt;
  }
  return midiFile;
}

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

std::vector<std::string> getTrackNames(const juce::MidiFile &midiFile) {
  std::vector<std::string> tracks;
  for (auto i = 0; i < midiFile.getNumTracks(); ++i) {
    const auto track = midiFile.getTrack(i);
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

float extractCrotchetsPerSecond(const juce::MidiFile &midiFile) {
  // This makes the assumption that the tempo stays constant.
  // For tempo-changing pieces, something more elaborate would be needed.
  // But again, this is only for the built-in tick provider ; a proper host
  // would do that.
  const auto secondsPerCrotchet = getFirstFromTrack<float>(
      *midiFile.getTrack(0),
      [](const juce::MidiMessage &msg) { return msg.isTempoMetaEvent(); },
      [](const juce::MidiMessage &msg) {
        return msg.getTempoSecondsPerQuarterNote();
      });
  if (secondsPerCrotchet.has_value()) {
    return 1 / *secondsPerCrotchet;
  } else {
    // TODO error
    return 0.f;
  }
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
    msgs.push_back({tick, msg.isNoteOn(), msg.getNoteNumber()});
    lastMsg = &msgs.back();
  }
  std::sort(msgs.begin(), msgs.end(),
            [](const MidiNoteMsg &a, const MidiNoteMsg &b) {
              return a.isNoteOn && !b.isNoteOn;
            });
  std::sort(msgs.begin(), msgs.end(),
            [](const MidiNoteMsg &a, const MidiNoteMsg &b) {
              return a.tick < b.tick;
            });
  auto it = std::prev(msgs.end());
  while (it != msgs.begin()) {
    auto prev = std::prev(it);
    if (prev->isNoteOn && !it->isNoteOn && prev->tick == it->tick) {
      msgs.erase(it);
    }
    it = prev;
  }
  return msgs;
}
} // namespace

void IntervalGetterFactory::setSampleRate(int sampleRate) {
  _samplesPerSecond = sampleRate;
}

void IntervalGetterFactory::setUseHostPlayhead(bool useHostPlayhead) {
  _useHostPlayhead = useHostPlayhead;
}

bool IntervalGetterFactory::getUseHostPlayhead() const {
  return _useHostPlayhead;
}

void IntervalGetterFactory::setMidiFile(std::filesystem::path path) {
  _juceMidiFile = getJuceMidiFile(path.string());
  _midiFilePath = std::move(path);
  _trackNames = _juceMidiFile ? getTrackNames(*_juceMidiFile)
                              : std::vector<std::string>{};
  _ticksPerCrotchet =
      _juceMidiFile
          ? std::optional<int>{static_cast<int>(_juceMidiFile->getTimeFormat())}
          : std::optional<int>{};
  _crotchetsPerSecond = _juceMidiFile
                            ? extractCrotchetsPerSecond(*_juceMidiFile)
                            : std::optional<float>{};

  _createIntervalGetterIfAllParametersSet();
}

std::optional<std::filesystem::path>
IntervalGetterFactory::getMidiFile() const {
  return _midiFilePath;
}

std::vector<std::string> IntervalGetterFactory::getMidiFileTrackNames() const {
  return _trackNames;
}

void IntervalGetterFactory::setPlayedTrack(int track) {
  if (_playedTrack != track) {
    _playedTrack = track;
    _createIntervalGetterIfAllParametersSet();
  }
}

std::optional<int> IntervalGetterFactory::getPlayedTrack() const {
  return _playedTrack;
}

void IntervalGetterFactory::setHarmonyTrack(int track) {
  if (_harmonyTrack != track) {
    _harmonyTrack = track;
    _createIntervalGetterIfAllParametersSet();
  }
}

std::optional<int> IntervalGetterFactory::getHarmonyTrack() const {
  return _harmonyTrack;
}

const std::vector<uint8_t> &IntervalGetterFactory::getState() const {
  return _state;
}

void IntervalGetterFactory::setState(std::vector<uint8_t>) {}

bool IntervalGetterFactory::hasIntervalGetter() const {
  return _intervalGetter.use_count() > 0;
}

std::shared_ptr<IntervalGetter>
IntervalGetterFactory::getIntervalGetter() const {
  return _intervalGetter;
}

bool IntervalGetterFactory::useHostPlayhead() const {
  return getUseHostPlayhead();
}

void IntervalGetterFactory::_createIntervalGetterIfAllParametersSet() {
  if (!_juceMidiFile || !_ticksPerCrotchet || !_playedTrack || !_harmonyTrack ||
      !_samplesPerSecond) {
    return;
  }
  const auto playedSeq = getMidiNoteMessages(
      *_juceMidiFile->getTrack(*_playedTrack), *_ticksPerCrotchet);
  const auto harmoSeq = getMidiNoteMessages(
      *_juceMidiFile->getTrack(*_harmonyTrack), *_ticksPerCrotchet);
  _intervalGetterInput = toIntervalSpans(playedSeq, harmoSeq);
  if (_intervalGetterInput.empty()) {
    // _logger->warn("toIntervalGetterInput returned empty vector");
  } else {
    const auto ticksPerSample =
        *_ticksPerCrotchet * *_crotchetsPerSecond / *_samplesPerSecond;
    // TODO: No need to wrap IntervalGetter
    if (utils::getEnvironmentVariableAsBool("SAINT_DEBUG_INTERVALGETTER") &&
        utils::isDebugBuild()) {
      _intervalGetter = std::make_shared<IntervalGetter>(
          _intervalGetterInput, *_ticksPerCrotchet, ticksPerSample,
          testUtils::getIntervalGetterDebugCb());
    } else {
      _intervalGetter = std::make_shared<IntervalGetter>(
          _intervalGetterInput, *_ticksPerCrotchet, ticksPerSample,
          std::nullopt);
    }
  }
}

std::optional<int> IntervalGetterFactory::getTicksPerCrotchet() const {
  return _ticksPerCrotchet;
}

std::optional<float> IntervalGetterFactory::getCrotchetsPerSecond() const {
  return _crotchetsPerSecond;
}

} // namespace saint
