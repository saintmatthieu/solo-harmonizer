#include "IntervalTypes.h"

namespace saint {
bool operator==(const PlayedNote &a, const PlayedNote &b) {
  return a.noteNumber == b.noteNumber && a.interval == b.interval;
}

bool operator==(const IntervalSpan &a, const IntervalSpan &b) {
  return a.playedNote == b.playedNote && a.beginTick == b.beginTick;
}

bool operator!=(const PlayedNote &a, const PlayedNote &b) { return !(a == b); }

bool operator!=(const IntervalSpan &a, const IntervalSpan &b) {
  return !(a == b);
}

} // namespace saint