#pragma once

#include "CommonTypes.h"
#include "KeyRecognizer.h"
#include "PitchMapper.h"

namespace saint {
class DefaultPitchMapper : public PitchMapper {
public:
  DefaultPitchMapper(const std::vector<IntervalSpan> &,
                     const std::map<float, Fraction> &);

  std::optional<float> getHarmony(float semiFromA, float crotchet) override;

private:
  const std::vector<IntervalSpan> &_spans;
  const std::map<float, Fraction> &_timeSignatures;
  KeyRecognizer _keyRecognizer;
  size_t _spanIndex = 0u;
};
} // namespace saint
