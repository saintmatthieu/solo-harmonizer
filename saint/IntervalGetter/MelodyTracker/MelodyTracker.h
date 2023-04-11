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
  // return value: note index, or nullopt if no current note index can be
  // assumed.
  virtual size_t onNoteOnSample(const std::chrono::milliseconds &now,
                                float noteNum) = 0;
  virtual void onNoteOff() = 0;
  virtual ~MelodyTracker() = default;
};
} // namespace saint