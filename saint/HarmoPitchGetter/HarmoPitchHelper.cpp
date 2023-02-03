#include "HarmoPitchHelper.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

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
} // namespace

std::vector<InputTimeEntry>
toHarmoPitchGetterInput(const std::filesystem::path &xmlConfig) {
  const juce::File file(xmlConfig.c_str());
  const auto xml = juce::XmlDocument::parse(file);
  const auto masterMidi = getMidiFile(xml->getAttributeValue(0));
  const auto slaveMidi = getMidiFile(xml->getAttributeValue(1));
  const juce::MidiMessageSequence *masterSequence = masterMidi.getTrack(1);
  std::vector<double> timestamps;
  std::vector<std::optional<int>> noteNumbers;
  std::optional<double> currentNoteoffTimestamp;
  for (auto it = masterSequence->begin(); it != masterSequence->end(); ++it) {
    const auto message = (*it)->message;
    if (!message.isNoteOn()) {
      continue;
    }
    const auto timestamp = message.getTimeStamp();
    if (currentNoteoffTimestamp && timestamp > currentNoteoffTimestamp) {
      timestamps.push_back(*currentNoteoffTimestamp);
      noteNumbers.push_back(std::nullopt);
      currentNoteoffTimestamp = std::nullopt;
    }
    timestamps.push_back(message.getTimeStamp());
    noteNumbers.push_back(message.getNoteNumber());
  }
}
} // namespace saint