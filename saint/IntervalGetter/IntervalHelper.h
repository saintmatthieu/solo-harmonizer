#pragma once

#include "IntervalTypes.h"

#include <vector>

namespace saint {
bool setIntervalIndex(const std::vector<int> &intervals, int &currentIndex,
                      double tick);

std::vector<IntervalSpan>
toIntervalSpans(const std::vector<MidiNoteMsg> &playedMidiTrack,
                const std::vector<MidiNoteMsg> &harmoMidiTrack);
} // namespace saint