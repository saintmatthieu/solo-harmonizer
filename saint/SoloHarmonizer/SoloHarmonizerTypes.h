#pragma once

namespace saint {
struct AudioConfig {
  int samplesPerSecond = 0;
  int samplesPerBlock = 0;
  int ticksPerCrotchet = 0;
  float crotchetsPerSecond = 0;
};

enum class TrackType { played, harmony, _size };
constexpr auto numTrackTypes = static_cast<size_t>(TrackType::_size);
constexpr auto playedTrackTypeIndex = static_cast<int>(TrackType::played);
constexpr auto harmonyTrackTypeIndex = static_cast<int>(TrackType::harmony);

} // namespace saint
