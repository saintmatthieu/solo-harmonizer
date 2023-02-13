#pragma once

#include <filesystem>
#include <set>

namespace saint {

struct TrackInfo {
  std::string name;
};

enum class TrackType { played, harmony, _size };
constexpr auto numTrackTypes = static_cast<size_t>(TrackType::_size);

class IGuiListener {
public:
  // Returns number of MIDI tracks.
  virtual std::vector<TrackInfo>
  onMidiFileChosen(const std::filesystem::path &) = 0;
  virtual void onTrackSelected(TrackType, int trackNumber) = 0;
  virtual ~IGuiListener() = default;
};
} // namespace saint
