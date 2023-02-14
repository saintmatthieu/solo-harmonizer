#pragma once

#include <optional>

namespace saint {
class ITicker {
public:
  virtual void incrementBlockCount() {}
  virtual std::optional<int> getTick() const = 0;
  virtual ~ITicker() = default;
};
} // namespace saint