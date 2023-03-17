#pragma once

#include "CommonTypes.h"

#include <vector>

namespace saint {
class PositionGetter {
public:
  PositionGetter(std::vector<TimeSignaturePosition> timeSignatures);
  Position getPosition(float timeInCrotchets) const;
  float getBarTimeInCrotchets(int barIndex) const;

private:
  const std::vector<TimeSignaturePosition> _timeSignaturePositions;
};
} // namespace saint
