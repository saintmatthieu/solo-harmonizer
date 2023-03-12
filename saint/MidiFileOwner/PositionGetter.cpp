#include "PositionGetter.h"
#include <tuple>

namespace saint {
namespace {
int getBeatsPerBar(const Fraction &timeSignature) {
  if (timeSignature.num > 3 && timeSignature.num % 3 == 0) {
    return timeSignature.num / 3;
  } else {
    return timeSignature.num;
  }
}
} // namespace

PositionGetter::PositionGetter(
    std::vector<TimeSignaturePosition> timeSignaturePositions)
    : _timeSignaturePositions(std::move(timeSignaturePositions)) {}

Position PositionGetter::getPosition(float crotchet) const {
  const auto sigBarIt = std::prev(std::find_if(
      _timeSignaturePositions.begin(), _timeSignaturePositions.end(),
      [crotchet](const TimeSignaturePosition &pos) {
        return pos.crotchet > crotchet;
      }));
  const auto crotchetsFromSigBar = sigBarIt->crotchet - crotchet;
  const auto sigBar = sigBarIt->barIndex + 1;
  const auto &sig = sigBarIt->timeSignature;
  const auto crotchetsPerBar = 4.f * sig.num / sig.den;
  const auto barsFromSigBar =
      static_cast<int>(crotchetsFromSigBar / crotchetsPerBar);
  const auto crotchetsFromBar =
      crotchetsFromSigBar - barsFromSigBar * crotchetsPerBar;
  const auto beatsPerBar = getBeatsPerBar(sig);
  const auto beatsPerCrotchet = beatsPerBar / crotchetsPerBar;
  return {sigBar + barsFromSigBar, beatsPerCrotchet * crotchetsFromBar};
}
} // namespace saint