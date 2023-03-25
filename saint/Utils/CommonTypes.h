#pragma once

#include <optional>
#include <vector>

namespace juce {
class MidiFile;
class MidiMessage;
class MidiMessageSequence;
} // namespace juce

namespace saint {

class IntervalGetter;
class PositionGetter;

enum class PlayheadCommand {
  play,
  pause,
  stop,
};

enum class Mode { minor, major };

enum class PC { // Pitch Class
  Fsh = 6,
  Csh,
  Gsh,
  Dsh,
  Ash,
  Esh,
  C = 0,
  G,
  D,
  A,
  E,
  B,
  Gb,
  Db,
  Ab,
  Eb,
  Bb,
  F,
};

struct PlayedNote {
  int noteNumber;
  // If not present then no harmonization.
  std::optional<int> interval = std::nullopt;
};

struct IntervalSpan {
  float beginCrotchet;
  std::optional<PlayedNote> playedNote;
  std::vector<int> overlappingNotes;
};

bool operator==(const PlayedNote &a, const PlayedNote &b);
bool operator==(const IntervalSpan &a, const IntervalSpan &b);
bool operator!=(const PlayedNote &a, const PlayedNote &b);
bool operator!=(const IntervalSpan &a, const IntervalSpan &b);

struct MidiNoteMsg {
  float crotchet;
  bool isNoteOn;
  int noteNumber;
};

struct Fraction {
  int num;
  int den;
};
bool operator==(const Fraction &, const Fraction &);
bool operator!=(const Fraction &, const Fraction &);

struct Position {
  int barIndex;
  float beatIndex;
};
bool operator==(const Position &, const Position &);
bool operator!=(const Position &, const Position &);

struct SigPos {
  int barIndex;
  Fraction timeSignature;
  bool operator==(const SigPos &) const;
  bool operator!=(const SigPos &) const;
};

struct SigPosWithCrotchet : SigPos {
  float crotchet;
};

struct Key {
  PC pc;
  Mode mode;
  bool operator==(const Key &) const;
  bool operator!=(const Key &) const;
};
} // namespace saint
