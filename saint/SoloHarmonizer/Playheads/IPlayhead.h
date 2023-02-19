#pragma once

#include <optional>

namespace saint {
class IPlayhead {
public:
  virtual void incrementSampleCount(int) {}
  virtual std::optional<double> getTimeInCrotchets() const = 0;
  virtual ~IPlayhead() = default;
};
} // namespace saint