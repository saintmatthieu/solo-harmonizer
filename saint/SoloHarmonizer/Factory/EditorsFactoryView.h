#pragma once

#include <filesystem>
#include <optional>

namespace saint {
class EditorsFactoryView {
public:
  virtual ~EditorsFactoryView() = default;
  virtual void setUseHostPlayhead(bool) = 0;
  virtual bool getUseHostPlayhead() const = 0;
  virtual void setMidiFile(std::filesystem::path) = 0;
  virtual std::optional<std::filesystem::path> getMidiFile() const = 0;
  virtual std::vector<std::string> getMidiFileTrackNames() const = 0;
  virtual void setPlayedTrack(int) = 0;
  virtual std::optional<int> getPlayedTrack() const = 0;
  virtual void setHarmonyTrack(int) = 0;
  virtual std::optional<int> getHarmonyTrack() const = 0;
};
} // namespace saint
