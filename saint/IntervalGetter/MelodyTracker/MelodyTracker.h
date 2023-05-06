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

  virtual void onHostTimeJump(float newTime) = 0;
  virtual void onNoteOnSample(const std::chrono::milliseconds &now,
                              float noteNum) = 0;
  // return value: note index, or nullopt if no current note index can be
  // assumed.
  virtual std::optional<size_t> onNoteOff() = 0;
  virtual ~MelodyTracker() = default;
};
} // namespace saint