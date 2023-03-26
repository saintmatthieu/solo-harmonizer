#pragma once

#include "CommonTypes.h"

namespace saint {
class DefaultPitchMapperHelper {
public:
  static float harmonize(float actualSemi, int intendedSemi, int harmonySemi,
                         const Key &key);
  static float
  harmonized(float semitoneFromC, const Key &key,
             float harmonizationDegree // 0 for unison, 1 for 2nd, 2 for 3rd,
                                       // ..., 7 for octave, etc.
  );

  static float semiToAeolianDegree(float semi, const Key &key);
  static float aeolianDegreeToSemi(float degree, const Key &key);
};
} // namespace saint
