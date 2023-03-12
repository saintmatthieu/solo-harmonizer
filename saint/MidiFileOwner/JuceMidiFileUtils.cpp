#include "JuceMidiFileUtils.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace saint {
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
} // namespace

std::optional<juce::MidiFile> getJuceMidiFile(const std::string &filename) {
  juce::MidiFile midiFile;
  const juce::File file(filename);
  const auto stream = file.createInputStream();
  if (!midiFile.readFrom(*stream)) {
    return std::nullopt;
  }
  return midiFile;
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

int getTicksPerCrotchet(const juce::MidiFile &midiFile) {
  return static_cast<int>(midiFile.getTimeFormat());
}

std::vector<TimeSignaturePosition>
getTimeSignatures(const juce::MidiFile &midiFile) {
  std::vector<TimeSignaturePosition> positions;
  auto barIndex = 0;
  auto barCrotchet = 0.f;
  auto crotchetsPerBar = 4.f;
  const juce::MidiMessageSequence *firstTrack = midiFile.getTrack(0);
  const auto ticksPerCrotchet = getTicksPerCrotchet(midiFile);
  for (auto it = firstTrack->begin(); it != firstTrack->end(); ++it) {
    const juce::MidiMessage &msg = (*it)->message;
    if (!msg.isTimeSignatureMetaEvent()) {
      continue;
    }
    TimeSignaturePosition position;
    position.crotchet = std::roundf(static_cast<float>(msg.getTimeStamp()) /
                                    static_cast<float>(ticksPerCrotchet) * 4) /
                        4;
    msg.getTimeSignatureInfo(position.timeSignature.num,
                             position.timeSignature.den);
    barIndex +=
        static_cast<int>((position.crotchet - barCrotchet) / crotchetsPerBar);
    position.barIndex = barIndex;
    crotchetsPerBar =
        4.f * position.timeSignature.num / position.timeSignature.den;
    barCrotchet = position.crotchet;
    positions.push_back(position);
  }
  return positions;
}

std::optional<std::vector<TimeSignaturePosition>>
getTimeSignatures(const std::string &filename) {
  const auto midiFile = getJuceMidiFile(filename);
  if (!midiFile) {
    return std::nullopt;
  }
  return getTimeSignatures(*midiFile);
}

} // namespace saint
