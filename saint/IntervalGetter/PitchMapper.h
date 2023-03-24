#pragma once

#include "CommonTypes.h"

#include <map>
#include <memory>
#include <vector>

namespace saint {
class PitchMapper {
public:
  static std::unique_ptr<PitchMapper>
  createInstance(const std::vector<IntervalSpan> &,
                 const std::map<float, Fraction> &timeSignatures);

  virtual std::optional<float>
  getHarmony(float semiFromA,
             const std::vector<IntervalSpan>::const_iterator &) = 0;

  virtual ~PitchMapper() = default;
};
} // namespace saint
