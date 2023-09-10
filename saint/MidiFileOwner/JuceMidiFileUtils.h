#pragma once

#include "CommonTypes.h"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace saint {
std::optional<juce::MidiFile> getJuceMidiFile(const std::string &filename);

std::map<int, std::string> getTrackNames(const juce::MidiFile &midiFile);

float extractCrotchetsPerSecond(const juce::MidiFile &midiFile);

std::vector<MidiNoteMsg> getMidiNoteMessages(const juce::MidiFile &, int track);

int getTicksPerCrotchet(const juce::MidiFile &midiFile);

std::vector<TimeSignaturePosition>
getTimeSignatures(const juce::MidiFile &midiFile);

std::optional<std::vector<TimeSignaturePosition>>
getTimeSignatures(const std::string &filename);
} // namespace saint
