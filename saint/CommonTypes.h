#pragma once

#include <filesystem>
#include <optional>

namespace saint {
struct Config {
  std::optional<std::filesystem::path> midiFilePath;
  std::optional<int> playedTrackNumber;
  std::optional<int> harmonyTrackNumber;
};
} // namespace saint