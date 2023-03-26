#pragma once

#include "CommonTypes.h"

#include <vector>

namespace saint {
std::optional<int> getClosestLimitIndex(const std::vector<float> &intervals,
                                        float crotchet);

std::vector<IntervalSpan>
toIntervalSpans(const std::vector<MidiNoteMsg> &playedMidiTrack,
                const std::vector<MidiNoteMsg> &harmoMidiTrack,
                const std::vector<Note> &allNotes);
} // namespace saint