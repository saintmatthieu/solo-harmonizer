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

std::vector<HarmoNoteSpan> toHarmoPitchGetterInput(const fs::path &xmlConfig) {
  const juce::File file(fs::absolute(xmlConfig).c_str());
  const auto xml = juce::XmlDocument::parse(file);
  const auto pathAttribute = xml->getStringAttribute("path").toStdString();
  const auto midiFilePath = fs::absolute(pathAttribute);
  if (!fs::exists(midiFilePath)) {
    // TODO report error
    return {};
  }
  const auto midiFile = getMidiFile(midiFilePath.string());
  const auto ticksPerSecond = midiFile.getTimeFormat();
  if (ticksPerSecond <= 0) {
    // TODO: report that SMPTE format is not supported yet
    return {};
  }

  // The following shoud be looped over each child element.
  const auto harmoEl = xml->getChildElement(0); // For now just using the first.
  if (!harmoEl->hasAttribute("playedTrack") ||
      !harmoEl->hasAttribute("harmonyTrack")) {
    return {};
  }
  const auto playedTrack = harmoEl->getIntAttribute("playedTrack");
  const auto harmonyTrack = harmoEl->getIntAttribute("harmonyTrack");
  const auto playedSeq = getMidiNoteMessages(*midiFile.getTrack(playedTrack),
                                             midiFile.getTimeFormat());
  const auto harmoSeq = getMidiNoteMessages(*midiFile.getTrack(harmonyTrack),
                                            midiFile.getTimeFormat());
  return toHarmoNoteSpans(playedSeq, harmoSeq);
}
} // namespace saint