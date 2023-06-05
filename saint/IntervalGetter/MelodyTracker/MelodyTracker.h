#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <vector>

namespace saint {
class MelodyTracker {
public:
  static std::unique_ptr<MelodyTracker> createInstance(
      const std::vector<std::pair<float, std::optional<int>>> &melody);
  virtual std::optional<size_t>
  tick(const std::optional<float> &measuredNoteNumber) = 0;
  virtual ~MelodyTracker() = default;
};
} // namespace saint