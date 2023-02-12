#include "HarmoPitchFileReader.h"
#include "HarmoPitchHelper.h"
#include "HarmoPitchTypes.h"

#include <filesystem>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include <cassert>
#include <optional>

namespace fs = std::filesystem;

namespace saint {
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

void quantizeTicks(juce::MidiMessageSequence &seq, int ticksPerCrotchet) {}

int getQuantizedTimestamp(double ts, double ticksPer32nd) {
  return static_cast<int>(ts / ticksPer32nd + 0.5) * ticksPer32nd;
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

std::vector<HarmoNoteSpan> toHarmoPitchGetterInput(const Config &config,
                                                   int *ticksPerCrotchet) {

  if (!config.harmonyTrackNumber.has_value() ||
      !config.playedTrackNumber.has_value() ||
      !config.midiFilePath.has_value() || !fs::exists(*config.midiFilePath)) {
    // TODO report error
    return {};
  }
  const auto midiFile = getMidiFile(config.midiFilePath->string());
  const auto ticksPerCrotchetShort = midiFile.getTimeFormat();
  if (ticksPerCrotchetShort <= 0) {
    // TODO: report that SMPTE format is not supported yet
    return {};
  } else if (ticksPerCrotchet != nullptr) {
    *ticksPerCrotchet = ticksPerCrotchetShort;
  }

  const auto playedSeq = getMidiNoteMessages(
      *midiFile.getTrack(*config.playedTrackNumber), midiFile.getTimeFormat());
  const auto harmoSeq = getMidiNoteMessages(
      *midiFile.getTrack(*config.harmonyTrackNumber), midiFile.getTimeFormat());
  return toHarmoNoteSpans(playedSeq, harmoSeq);
}
} // namespace saint