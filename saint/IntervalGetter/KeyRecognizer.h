#pragma once

#include "CommonTypes.h"

#include <map>

namespace saint {
class KeyRecognizer {
public:
  KeyRecognizer(const std::vector<IntervalSpan> &spans,
                const std::map<float, Fraction> &timeSignatures);
  Key getKey(float crotchet);

private:
  const std::vector<IntervalSpan> &_spans;
  const std::vector<std::pair<size_t /*end index */, Key>> _spanKeys;
  size_t _spanIndex = 0u;
};
} // namespace saint
