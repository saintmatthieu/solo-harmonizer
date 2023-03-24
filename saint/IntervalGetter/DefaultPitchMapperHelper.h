#pragma once

namespace saint {
enum class PC { // Pitch Class
  Gb = -6,
  Db,
  Ab,
  Eb,
  Bb,
  F,
  C,
  G,
  D,
  A,
  E,
  B,
  Fsh,
  Csh,
  Gsh,
  Dsh,
  Ash
};

enum class Mode { minor, major };

struct Key {
  const PC pc;
  const Mode mode;
};

class DefaultPitchMapperHelper {
public:
  static float harmonize(float actualSemi, int intendedSemi, int harmonySemi,
                         const Key &key);
  static float
  harmonized(float semitoneFromA, const Key &key,
             int harmonizationDegree // 0 for unison, 1 for 2nd, 2 for 3rd, ...,
                                     // 7 for octave, etc.
  );
};
} // namespace saint
