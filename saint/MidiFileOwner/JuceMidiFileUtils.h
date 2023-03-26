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

std::vector<MidiNoteMsg> parseMonophonicTrack(const juce::MidiFile &,
                                              int track);

std::vector<Note> getNotes(const juce::MidiFile &,
                           const std::vector<int> &tracksToExclude = {});

int getTicksPerCrotchet(const juce::MidiFile &midiFile);

std::vector<SigPosWithCrotchet>
getTimeSignatures(const juce::MidiFile &midiFile);

std::map<float, Fraction> getTimeSignatureMap(const juce::MidiFile &midiFile);

std::optional<std::vector<SigPosWithCrotchet>>
getTimeSignatures(const std::string &filename);
} // namespace saint
