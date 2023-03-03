#include "IntervalHelper.h"

#include <cassert>
#include <iterator>

namespace saint {
std::optional<int> getClosestLimitIndex(const std::vector<int> &intervals,
                                        double tick) {
  if (intervals.size() < 2u) {
    return false;
  }

  if (tick < intervals[0]) {
    // apply round-up rule
    const auto leftLimit = intervals[0];
    const auto rightLimit = intervals[1];
    const auto snapRange = (rightLimit - leftLimit) / 2.f;
    if (tick >= leftLimit - snapRange) {
      return 0;
    } else {
      return std::nullopt;
    }
  }

  const auto lastRightLimitIt = std::prev(intervals.end());
  auto leftLimitIt = intervals.begin();
  while (leftLimitIt != lastRightLimitIt) {
    if (*leftLimitIt <= tick && tick < *std::next(leftLimitIt)) {
      break;
    }
    ++leftLimitIt;
  }

  if (leftLimitIt == lastRightLimitIt) {
    return std::nullopt;
  }

  const auto leftLimit = *leftLimitIt;
  const auto rightLimit = *std::next(leftLimitIt);
  const auto leftLimitIndex = std::distance(intervals.begin(), leftLimitIt);
  const auto closestLimitIndex = tick - leftLimit < rightLimit - tick
                                     ? leftLimitIndex
                                     : leftLimitIndex + 1;
  return closestLimitIndex == static_cast<int>(intervals.size() - 1)
             ? std::optional<int>{}
             : std::optional<int>{closestLimitIndex};
}

std::vector<IntervalSpan>
toIntervalSpans(const std::vector<MidiNoteMsg> &playedMidiTrack,
                const std::vector<MidiNoteMsg> &harmoMidiTrack) {
  std::vector<IntervalSpan> spans;
  auto playing = false;
  auto harmoIt = harmoMidiTrack.begin();
  for (auto playedIt = playedMidiTrack.begin();
       playedIt != playedMidiTrack.end(); ++playedIt) {
    const auto &played = *playedIt;
    if (!played.isNoteOn) {
      assert(playing); // Looks like this isn't a monophonic track
      if (playing) {
        spans.push_back({played.tick, std::nullopt});
        playing = false;
      }
      continue;
    }
    playing = true;
    if (spans.size() > 0u && !spans.back().playedNote &&
        spans.back().beginTick == played.tick) {
      // This new NoteOn coincides with the previous NoteOff => let's delete
      // that NoteOff
      spans.pop_back();
    }
    spans.push_back({played.tick, PlayedNote{played.noteNumber, std::nullopt}});
    // Look for harmonized note at that tick ...
    harmoIt = std::find_if(harmoIt, harmoMidiTrack.end(),
                           [&played](const MidiNoteMsg &harmo) {
                             return harmo.isNoteOn && harmo.tick >= played.tick;
                           });
    if (harmoIt == harmoMidiTrack.end()) {
      // ... no harmonized note anymore.
      continue;
    }
    const auto &harmonyMsg = (*harmoIt);
    if (harmonyMsg.tick > played.tick) {
      // Looks like played and harmony are not homorythmic - TODO issue a
      // warning ?
      continue;
    } else {
      // Get note value
      spans.back().playedNote->interval =
          harmonyMsg.noteNumber - played.noteNumber;
    }
  }
  if (spans.size() > 0 && spans[0].beginTick > 0) {
    spans.insert(spans.begin(), {0, std::nullopt});
  }
  return spans;
}
} // namespace saint
