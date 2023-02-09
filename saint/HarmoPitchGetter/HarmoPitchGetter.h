#pragma once

#include "HarmoPitchTypes.h"

#include <vector>

namespace saint {
class HarmoPitchGetter {
public:
  HarmoPitchGetter(const std::vector<HarmoNoteSpan> &timeSegments);
  std::optional<float> intervalInSemitonesAtTick(int tick);

private:
  const std::vector<Bkpt> _bkpts;
  std::vector<Bkpt>::const_iterator _bkptsIt;
};
} // namespace saint
