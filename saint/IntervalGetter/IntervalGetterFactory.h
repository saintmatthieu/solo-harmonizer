#pragma once

#include "CommonTypes.h"

#include <map>
#include <memory>
#include <optional>

namespace saint {
class IntervalGetter;
class IntervalGetterFactory {
public:
  virtual ~IntervalGetterFactory() = default;

  virtual std::shared_ptr<IntervalGetter>
  createInstance(const std::vector<IntervalSpan> &spans,
                 const std::map<float, Fraction> &timeSignatures,
                 const std::optional<int> &samplesPerSecond,
                 const std::optional<float> &crotchetsPerSecond) const = 0;
};
} // namespace saint