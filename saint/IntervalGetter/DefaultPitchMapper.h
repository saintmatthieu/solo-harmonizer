#pragma once

#include "CommonTypes.h"
#include "KeyRecognizer.h"
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
  const std::map<float, Fraction> &_timeSignatures;
  KeyRecognizer _keyRecognizer;
};
} // namespace saint
