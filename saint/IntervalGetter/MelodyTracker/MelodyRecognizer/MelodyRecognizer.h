#pragma once

#include <chrono>
#include <optional>
#include <vector>

namespace saint {
class MelodyRecognizer {
public:
  virtual bool onNoteOff(
      const std::vector<std::pair<std::chrono::milliseconds, float>> &) = 0;
  virtual size_t getNextNoteIndex() = 0;
  virtual ~MelodyRecognizer() = default;
};
} // namespace saint