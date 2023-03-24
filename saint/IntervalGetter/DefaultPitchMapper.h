#pragma once

#include "CommonTypes.h"
#include "PitchMapper.h"

namespace saint {
class DefaultPitchMapper : public PitchMapper {
public:
  DefaultPitchMapper(const std::vector<IntervalSpan> &,
                     const std::map<float, Fraction> &);

  std::optional<float>
  getHarmony(float semiFromA,
             const std::vector<IntervalSpan>::const_iterator &) override;

private:
  const std::vector<IntervalSpan> &_spans;
  const std::map<float, Fraction> &_timeSignatures;
};
} // namespace saint
