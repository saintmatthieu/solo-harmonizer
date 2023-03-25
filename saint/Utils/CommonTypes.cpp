#include "CommonTypes.h"

#include <tuple>

namespace saint {
bool operator==(const PlayedNote &a, const PlayedNote &b) {
  return a.noteNumber == b.noteNumber && a.interval == b.interval;
}

bool operator==(const IntervalSpan &a, const IntervalSpan &b) {
  return a.playedNote == b.playedNote && a.beginCrotchet == b.beginCrotchet;
}

bool operator!=(const PlayedNote &a, const PlayedNote &b) { return !(a == b); }

bool operator!=(const IntervalSpan &a, const IntervalSpan &b) {
  return !(a == b);
}

bool operator==(const Fraction &lhs, const Fraction &rhs) {
  return std::tie(lhs.den, lhs.num) == std::tie(rhs.den, rhs.num);
}

bool operator!=(const Fraction &lhs, const Fraction &rhs) {
  return !(lhs == rhs);
}

bool operator==(const Position &lhs, const Position &rhs) {
  return std::tie(lhs.barIndex, lhs.beatIndex) ==
         std::tie(rhs.barIndex, rhs.beatIndex);
}

bool operator!=(const Position &lhs, const Position &rhs) {
  return !(lhs == rhs);
}

bool SigPos::operator==(const SigPos &other) const {
  return std::tie(barIndex, timeSignature) ==
         std::tie(other.barIndex, other.timeSignature);
}

bool SigPos::operator!=(const SigPos &other) const { return !(*this == other); }

bool Key::operator==(const Key &other) const {
  return std::tie(pc, mode) == std::tie(other.pc, other.mode);
}

bool Key::operator!=(const Key &other) const { return !(*this == other); }

} // namespace saint