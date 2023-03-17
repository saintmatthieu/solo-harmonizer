#pragma once

#include <optional>

namespace saint {
class Playhead {
public:
  // Should return new time in crotchets
  virtual std::optional<float> incrementSampleCount(int) = 0;
  virtual void mixMetronome(float *, int) {}
  virtual std::optional<float> getTimeInCrotchets() = 0;
  virtual ~Playhead() = default;
};
} // namespace saint