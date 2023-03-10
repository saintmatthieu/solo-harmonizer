#pragma once

#include "SoloHarmonizerTypes.h"

#include <optional>

namespace saint {
class Playhead {
public:
  virtual void incrementSampleCount(int) {}
  virtual void mixMetronome(float *, int) {}
  virtual std::optional<float> getTimeInCrotchets() const = 0;
  virtual ~Playhead() = default;
};
} // namespace saint