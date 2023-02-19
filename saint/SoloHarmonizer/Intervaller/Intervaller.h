#pragma once

#include <optional>

namespace saint {
class Intervaller {
public:
  virtual ~Intervaller() = default;
  virtual std::optional<float> getSemitoneInterval(double timeInCrotchets) = 0;
};
} // namespace saint
