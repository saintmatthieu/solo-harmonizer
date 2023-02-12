#pragma once

#include <filesystem>
#include <set>

namespace saint {
class IGuiListener {
public:
  virtual void onMidiFileChosen(const std::filesystem::path &) = 0;
  virtual bool isReady() const = 0;
  virtual std::set<int> getMidiTracks() const = 0;
  virtual ~IGuiListener() = default;
};
} // namespace saint
