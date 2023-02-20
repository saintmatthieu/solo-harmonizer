#include "HarmoPitchGetter.h"
#include "HarmoPitchGetterFactory.h"
#include "HarmoPitchHelper.h"
#include "HarmoPitchTypes.h"

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

float getCrotchetsPerSecond(const juce::MidiFile &midiFile) {
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
  if (secondsPerCrotchet.has_value()) {
    return static_cast<float>(1 / (*secondsPerCrotchet) + 0.5);
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

void HarmoPitchGetterFactory::setUseHostPlayhead(bool useHostPlayhead) {
  _useHostPlayhead = useHostPlayhead;
}

bool HarmoPitchGetterFactory::getUseHostPlayhead() const {
  return _useHostPlayhead;
}

void HarmoPitchGetterFactory::setMidiFile(std::filesystem::path path) {
  _juceMidiFile = getJuceMidiFile(path.string());
  _midiFilePath = std::move(path);
  _trackNames = _juceMidiFile ? getTrackNames(*_juceMidiFile)
                              : std::vector<std::string>{};
  _ticksPerCrotchet =
      _juceMidiFile
          ? std::optional<int>{static_cast<int>(_juceMidiFile->getTimeFormat())}
          : std::optional<int>{};
  _crotchetsPerSecond = _juceMidiFile ? getCrotchetsPerSecond(*_juceMidiFile)
                                      : std::optional<float>{};

  _prepareHarmoPitchGetterInputIfAllParametersSet();
}

std::optional<std::filesystem::path>
HarmoPitchGetterFactory::getMidiFile() const {
  return _midiFilePath;
}

std::vector<std::string>
HarmoPitchGetterFactory::getMidiFileTrackNames() const {
  return _trackNames;
}

void HarmoPitchGetterFactory::setPlayedTrack(int track) {
  if (_playedTrack != track) {
    _playedTrack = track;
    _prepareHarmoPitchGetterInputIfAllParametersSet();
  }
}

std::optional<int> HarmoPitchGetterFactory::getPlayedTrack() const {
  return _playedTrack;
}

void HarmoPitchGetterFactory::setHarmonyTrack(int track) {
  if (_harmonyTrack != track) {
    _harmonyTrack = track;
    _prepareHarmoPitchGetterInputIfAllParametersSet();
  }
}

std::optional<int> HarmoPitchGetterFactory::getHarmonyTrack() const {
  return _harmonyTrack;
}

const std::vector<uint8_t> &HarmoPitchGetterFactory::getState() const {
  return _state;
}

void HarmoPitchGetterFactory::setState(std::vector<uint8_t>) {}

bool HarmoPitchGetterFactory::hasHarmoPitchGetter() const {
  return _harmoPitchGetter.use_count() > 0;
}

std::shared_ptr<HarmoPitchGetter>
HarmoPitchGetterFactory::getHarmoPitchGetter() const {
  return _harmoPitchGetter;
}

bool HarmoPitchGetterFactory::useHostPlayhead() const {
  return getUseHostPlayhead();
}

void HarmoPitchGetterFactory::
    _prepareHarmoPitchGetterInputIfAllParametersSet() {
  if (!_juceMidiFile || !_ticksPerCrotchet || !_playedTrack || !_harmonyTrack) {
    return;
  }
  const auto playedSeq = getMidiNoteMessages(
      *_juceMidiFile->getTrack(*_playedTrack), *_ticksPerCrotchet);
  const auto harmoSeq = getMidiNoteMessages(
      *_juceMidiFile->getTrack(*_harmonyTrack), *_ticksPerCrotchet);
  _harmoPitchGetterInput = toHarmoNoteSpans(playedSeq, harmoSeq);
  if (_harmoPitchGetterInput.empty()) {
    // _logger->warn("toHarmoPitchGetterInput returned empty vector");
  } else {
    // TODO: No need to wrap HarmoPitchGetter
    _harmoPitchGetter = std::make_shared<HarmoPitchGetter>(
        _harmoPitchGetterInput, *_ticksPerCrotchet);
  }
}

} // namespace saint
