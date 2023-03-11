#pragma once

#include <vector>

namespace saint {
struct Fraction {
  int num;
  int den;
};

struct Position {
  int bar;
  Fraction WholeFraction;
};

struct TimeSignaturePosition {
  int bar;
  Fraction timeSignature;
};

class PositionGetter {
public:
  PositionGetter(std::vector<TimeSignaturePosition> timeSignatures);
  Position getPosition(float timeInCrotchets);
};
} // namespace saint
