#pragma once

#include "CommonTypes.h"

namespace saint {
class DefaultPitchMapperHelper {
public:
  static float harmonize(float actualNn, int intendedNn, int harmonyNn,
                         const Key &key);
  static float harmonized(float noteNumber, const Key &key,
                          float harmonizationDegree);
  static float nnToAeolianDegree(float noteNumber, const Key &key);
  static float aeolianDegreeToNn(float degree, const Key &key);
};
} // namespace saint
