#pragma once

#include "HarmoPitchTypes.h"

#include <vector>

namespace saint {
bool setIntervalIndex(const std::vector<int> &intervals, size_t *currentIndex,
                      double tick);

std::vector<HarmoNoteSpan>
toHarmoNoteSpans(const std::vector<MidiNoteMsg> &playedMidiTrack,
                 const std::vector<MidiNoteMsg> &harmoMidiTrack);
} // namespace saint