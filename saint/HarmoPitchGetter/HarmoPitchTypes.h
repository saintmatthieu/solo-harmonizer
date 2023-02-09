#pragma once

#include <optional>

namespace saint {
struct PlayedNote {
  int noteNumber;
  // If not present then no harmonization.
  std::optional<int> interval = std::nullopt;
};

struct HarmoNoteSpan {
  const int beginTick;
  std::optional<PlayedNote> playedNote;
};

struct Bkpt {
  const int tick;
  const std::optional<int> transposeSemitones;
};
} // namespace saint
