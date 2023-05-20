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

  virtual std::optional<size_t> beginNewNote(int tick) = 0;
  virtual void addPitchMeasurement(float pc) = 0;
  virtual ~MelodyTracker() = default;
};
} // namespace saint