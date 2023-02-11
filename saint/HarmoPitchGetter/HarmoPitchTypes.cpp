#include "HarmoPitchTypes.h"

namespace saint {
bool operator==(const PlayedNote &a, const PlayedNote &b) {
  return a.noteNumber == b.noteNumber && a.interval == b.interval;
}

bool operator==(const HarmoNoteSpan &a, const HarmoNoteSpan &b) {
  return a.playedNote == b.playedNote && a.beginTick == b.beginTick;
}

bool operator!=(const PlayedNote &a, const PlayedNote &b) { return !(a == b); }

bool operator!=(const HarmoNoteSpan &a, const HarmoNoteSpan &b) {
  return !(a == b);
}

} // namespace saint