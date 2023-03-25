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
    : _timeSignatures(timeSignatures), _keyRecognizer(spans, timeSignatures) {}

std::optional<float> DefaultPitchMapper::getHarmony(
    float semiFromA, const std::vector<IntervalSpan>::const_iterator &it) {
  if (!it->playedNote.has_value()) {
    return std::nullopt;
  }
  const auto &playedNote = *it->playedNote;
  if (!playedNote.interval.has_value()) {
    return std::nullopt;
  }
  const auto key = _keyRecognizer.getKey(it);
  const auto playedSemi = static_cast<float>(playedNote.noteNumber - 69);
  const auto harmoSemi = playedSemi + static_cast<float>(*playedNote.interval);
  return DefaultPitchMapperHelper::harmonize(semiFromA, playedSemi, harmoSemi,
                                             key);
}
} // namespace saint
