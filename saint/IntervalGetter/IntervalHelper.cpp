#include "IntervalHelper.h"
#include "CommonTypes.h"

#include <cassert>
#include <iterator>

namespace saint {
std::optional<int> getClosestLimitIndex(const std::vector<float> &intervals,
                                        float crotchet) {
  if (intervals.size() < 2u) {
    return false;
  }

  if (crotchet < intervals[0]) {
    // apply round-up rule
    const auto leftLimit = intervals[0];
    const auto rightLimit = intervals[1];
    const auto snapRange = (rightLimit - leftLimit) / 2.f;
    if (crotchet >= leftLimit - snapRange) {
      return 0;
    } else {
      return std::nullopt;
    }
  }

  const auto lastRightLimitIt = std::prev(intervals.end());
  auto leftLimitIt = intervals.begin();
  while (leftLimitIt != lastRightLimitIt) {
    if (*leftLimitIt <= crotchet && crotchet < *std::next(leftLimitIt)) {
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
  const auto closestLimitIndex = crotchet - leftLimit < rightLimit - crotchet
                                     ? leftLimitIndex
                                     : leftLimitIndex + 1;
  return closestLimitIndex == static_cast<int>(intervals.size() - 1)
             ? std::optional<int>{}
             : std::optional<int>{closestLimitIndex};
}

namespace {
void addOverlappingNotes(std::vector<IntervalSpan> &spans,
                         const std::vector<Note> &allNotes) {
  // Makes a big assumption: that `allNotes` are ordered by duration and then by
  // beginCrotchet.
  auto noteIt = allNotes.begin();
  auto spanIt = spans.begin();
  while (spanIt != std::prev(spans.end()) && noteIt != allNotes.end()) {
    const auto spanEnd = (spanIt + 1)->beginCrotchet;
    if (noteIt->beginCrotchet >= spanEnd) {
      ++spanIt;
      continue;
    } else if (spanIt->beginCrotchet >= noteIt->endCrotchet) {
      ++noteIt;
      continue;
    }
    spanIt->overlappingNotes.insert(noteIt->noteNumber);
    if (std::next(noteIt) == allNotes.end() ||
        spanEnd < std::next(noteIt)->beginCrotchet) {
      ++spanIt;
    } else {
      ++noteIt;
    }
  }
}
} // namespace

std::vector<IntervalSpan>
toIntervalSpans(const std::vector<MidiNoteMsg> &playedMidiTrack,
                const std::vector<MidiNoteMsg> &harmoMidiTrack,
                const std::vector<Note> &allNotes) {
  std::vector<IntervalSpan> spans;
  auto harmoIt = harmoMidiTrack.begin();
  for (auto playedIt = playedMidiTrack.begin();
       playedIt != playedMidiTrack.end(); ++playedIt) {
    const auto &played = *playedIt;
    if (!played.isNoteOn) {
      spans.push_back({played.crotchet, std::nullopt});
      continue;
    }
    if (spans.size() > 0u && !spans.back().playedNote &&
        spans.back().beginCrotchet == played.crotchet) {
      // This new NoteOn coincides with the previous NoteOff => let's delete
      // that NoteOff
      spans.pop_back();
    }
    spans.push_back(
        {played.crotchet, PlayedNote{played.noteNumber, std::nullopt}});
    // Look for harmonized note at that crotchet ...
    harmoIt = std::find_if(
        harmoIt, harmoMidiTrack.end(), [&played](const MidiNoteMsg &harmo) {
          return harmo.isNoteOn && harmo.crotchet >= played.crotchet;
        });
    if (harmoIt == harmoMidiTrack.end()) {
      // ... no harmonized note anymore.
      continue;
    }
    const auto &harmonyMsg = (*harmoIt);
    if (harmonyMsg.crotchet > played.crotchet) {
      // Looks like played and harmony are not homorythmic - TODO issue a
      // warning ?
      continue;
    } else {
      // Get note value
      spans.back().playedNote->interval =
          harmonyMsg.noteNumber - played.noteNumber;
    }
  }
  if (spans.size() > 0 && spans[0].beginCrotchet > 0) {
    spans.insert(spans.begin(), {0, std::nullopt});
  }
  addOverlappingNotes(spans, allNotes);
  return spans;
}
} // namespace saint
