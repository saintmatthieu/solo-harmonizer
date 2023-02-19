#pragma once

#include <optional>

namespace saint {
class Intervaller {
public:
  virtual ~Intervaller() = default;
  virtual std::optional<float> getSemitoneInterval(int tick) = 0;
  virtual int getTicksPerCrotchet() const = 0;
  virtual float getCrotchetsPerSecond() const = 0;
};
} // namespace saint
