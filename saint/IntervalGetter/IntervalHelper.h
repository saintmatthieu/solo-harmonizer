#pragma once

#include "IntervalTypes.h"

#include <vector>

namespace saint {
std::optional<int> getClosestLimitIndex(const std::vector<int> &intervals,
                                        double tick);

std::vector<IntervalSpan>
toIntervalSpans(const std::vector<MidiNoteMsg> &playedMidiTrack,
                const std::vector<MidiNoteMsg> &harmoMidiTrack);
} // namespace saint