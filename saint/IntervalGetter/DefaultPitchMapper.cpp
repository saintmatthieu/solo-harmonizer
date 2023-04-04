#include "DefaultPitchMapper.h"
#include "CommonTypes.h"
#include "DefaultPitchMapperHelper.h"

#include <cmath>

namespace saint {

std::unique_ptr<PitchMapper>
PitchMapper::createInstance(const std::vector<IntervalSpan> &spans,
                            const std::map<float, Fraction> &timeSignatures) {
  return std::make_unique<DefaultPitchMapper>(spans, timeSignatures);
}

DefaultPitchMapper::DefaultPitchMapper(
    const std::vector<IntervalSpan> &spans,
    const std::map<float, Fraction> &timeSignatures)
    : _spans(spans), _timeSignatures(timeSignatures),
      _keyRecognizer(spans, timeSignatures) {}

std::optional<float> DefaultPitchMapper::getHarmony(float perfNn,
                                                    float crotchet) {
  while (_spanIndex < _spans.size() &&
         _spans[_spanIndex].beginCrotchet <= crotchet) {
    ++_spanIndex;
  }
  --_spanIndex;
  const auto &span = _spans[_spanIndex];
  if (!span.playedNote.has_value()) {
    return std::nullopt;
  }
  const auto &playedNote = *span.playedNote;
  if (!playedNote.interval.has_value()) {
    return std::nullopt;
  }
  const auto key = _keyRecognizer.getKey(crotchet);
  const auto programmedHarmoNn = playedNote.noteNumber + *playedNote.interval;
  const auto performanceHarmoNn = DefaultPitchMapperHelper::harmonize(
      perfNn, playedNote.noteNumber, programmedHarmoNn, key);
  return performanceHarmoNn - 69.f;
}
} // namespace saint
