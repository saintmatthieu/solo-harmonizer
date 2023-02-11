#pragma once

#include <optional>

namespace saint {
struct PlayedNote {
  int noteNumber;
  // If not present then no harmonization.
  std::optional<int> interval = std::nullopt;
};

struct HarmoNoteSpan {
  int beginTick;
  std::optional<PlayedNote> playedNote;
};

bool operator==(const PlayedNote &a, const PlayedNote &b);
bool operator==(const HarmoNoteSpan &a, const HarmoNoteSpan &b);
bool operator!=(const PlayedNote &a, const PlayedNote &b);
bool operator!=(const HarmoNoteSpan &a, const HarmoNoteSpan &b);

struct MidiNoteMsg {
  const int tick; // quantized
  const bool isNoteOn;
  const int noteNumber;
};
} // namespace saint
