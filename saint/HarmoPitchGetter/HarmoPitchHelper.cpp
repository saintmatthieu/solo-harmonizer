#include "HarmoPitchHelper.h"

#include <cassert>
#include <iterator>

namespace saint {
bool setIntervalIndex(const std::vector<int> &intervals, size_t *pCurrentIndex,
                      int tick) {
  if (intervals.size() == 0u || pCurrentIndex == nullptr ||
      tick < intervals[0] || tick > intervals[intervals.size() - 1]) {
    return false;
  }
  auto &index = *pCurrentIndex;
  while (index < intervals.size() && intervals[index] <= tick) {
    ++index;
  }
  while (index == intervals.size() || intervals[index] > tick) {
    --index;
  }
  return true;
}

std::vector<HarmoNoteSpan>
toHarmoNoteSpans(const std::vector<MidiNoteMsg> &playedMidiTrack,
                 const std::vector<MidiNoteMsg> &harmoMidiTrack) {
  std::vector<HarmoNoteSpan> spans;
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
