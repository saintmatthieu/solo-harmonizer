#include "HarmoPitchGetter.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>

namespace saint {

namespace {
std::vector<Bkpt> toBkpts(const std::vector<HarmoNoteSpan> &spans) {
  std::vector<Bkpt> bkpts;
  if (spans.empty()) {
    return bkpts;
  }
  if (spans[0].beginTick > 0) {
    bkpts.push_back({0, std::nullopt});
  }
  std::transform(spans.begin(), spans.end(), std::back_inserter(bkpts),
                 [](const HarmoNoteSpan &span) -> Bkpt {
                   return {span.beginTick,
                           span.playedNote && span.playedNote->interval
                               ? *span.playedNote->interval
                               : std::optional<int>{}};
                 });
  return bkpts;
}
} // namespace

HarmoPitchGetter::HarmoPitchGetter(const std::vector<HarmoNoteSpan> &spans)
    : _bkpts(toBkpts(spans)), _bkptsIt(_bkpts.begin()) {}

std::optional<float> HarmoPitchGetter::intervalInSemitonesAtTick(int tick) {
  _bkptsIt = std::lower_bound(
      _bkptsIt, _bkpts.end(), tick,
      [](const Bkpt &bkpt, int tick) { return bkpt.tick <= tick; });
  const auto &end = _bkptsIt;
  const auto begin = std::prev(_bkptsIt);
  if (!begin->transposeSemitones) {
    return std::nullopt;
  } else if (end == _bkpts.end() || !end->transposeSemitones) {
    return static_cast<float>(*begin->transposeSemitones);
  } else {
    // Linear interpolation
    const auto x0 = begin->tick;
    const auto x1 = end->tick;
    const auto y0 = *begin->transposeSemitones;
    const auto y1 = *end->transposeSemitones;
    const auto xDiff = x1 - x0;
    const auto yDiff = y1 - y0;
    const auto offset = tick - x0;
    return static_cast<float>(offset) * yDiff / xDiff + y0;
  }
}

} // namespace saint