#include "PositionGetter.h"
#include "Utils.h"

#include <tuple>

namespace saint {
PositionGetter::PositionGetter(
    std::vector<SigPosWithCrotchet> timeSignaturePositions)
    : _timeSignaturePositions(std::move(timeSignaturePositions)) {}

Position PositionGetter::getPosition(float crotchet) const {
  return utils::getCrotchetPosition(crotchet, _timeSignaturePositions);
}

float PositionGetter::getBarTimeInCrotchets(int barIndex) const {
  const auto sigBarIt = std::prev(std::find_if(
      _timeSignaturePositions.begin(), _timeSignaturePositions.end(),
      [barIndex](const SigPosWithCrotchet &timeSig) {
        return timeSig.barIndex > barIndex;
      }));
  const auto barsFromSignBar = barIndex - sigBarIt->barIndex;
  const auto &sig = sigBarIt->timeSignature;
  const auto crotchetsPerBar = 4.f * sig.num / sig.den;
  return sigBarIt->crotchet + crotchetsPerBar * barsFromSignBar;
}
} // namespace saint