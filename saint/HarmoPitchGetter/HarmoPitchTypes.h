#pragma once

#include <optional>

namespace saint {
// Only minor modes - major modes would only be natural.
// So if the key is e.g. C Major, use A natural.
enum class Mode {
  natural,
  harmonic,
  melodic,
};

enum class PitchClass { A, B, C, D, E, F, G };

struct Note {
  PitchClass pitchClass;
  bool isSharp = false;
};

struct Key {
  Note note;
  Mode mode = Mode::natural;
};

struct ReferenceNote {
  Note value;
  // If not present then no harmonization.
  std::optional<Note> harmony = std::nullopt;
  // Maybe we can infer this ...
  std::optional<Key> key = std::nullopt;
};

struct InputTimeEntry {
  int tick;
  std::optional<ReferenceNote> reference;
};
} // namespace saint
