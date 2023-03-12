#pragma once

#include <functional>
#include <memory>
#include <optional>

namespace saint {

class JuceAudioPlayHeadProvider;
class Playhead;
class SoloHarmonizerEditor;

enum class TrackType { played, harmony, _size };

using PlayheadFactory = std::function<std::shared_ptr<Playhead>(
    bool mustSetPpqPosition, const JuceAudioPlayHeadProvider &playheadProvider,
    const std::optional<float> &crotchetsPerSecond,
    const std::optional<int> &samplesPerSecond)>;

constexpr auto numTrackTypes = static_cast<size_t>(TrackType::_size);
constexpr auto playedTrackTypeIndex = static_cast<int>(TrackType::played);
constexpr auto harmonyTrackTypeIndex = static_cast<int>(TrackType::harmony);

} // namespace saint
