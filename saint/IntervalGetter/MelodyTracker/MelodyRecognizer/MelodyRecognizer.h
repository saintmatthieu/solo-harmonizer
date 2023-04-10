#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <vector>

namespace saint {
class MelodyRecognizer {
public:
  static std::unique_ptr<MelodyRecognizer> createInstance(
      const std::vector<std::pair<float, std::optional<int>>> &melody);

  virtual bool onNoteOff(
      const std::vector<std::pair<std::chrono::milliseconds, float>> &) = 0;
  virtual size_t getNextNoteIndex() = 0;

  virtual ~MelodyRecognizer() = default;
};
} // namespace saint