#pragma once

#include <functional>
#include <memory>

namespace saint {

class JuceAudioPlayHeadProvider;
class Playhead;

enum class PlayheadCommand {
  play,
  pause,
  stop,
};

using PlayheadFactory = std::function<std::unique_ptr<Playhead>(
    bool mustSetPpqPosition, const JuceAudioPlayHeadProvider &playheadProvider,
    float crotchetsPerSample)>;

enum class TrackType { played, harmony, _size };
constexpr auto numTrackTypes = static_cast<size_t>(TrackType::_size);
constexpr auto playedTrackTypeIndex = static_cast<int>(TrackType::played);
constexpr auto harmonyTrackTypeIndex = static_cast<int>(TrackType::harmony);

} // namespace saint
