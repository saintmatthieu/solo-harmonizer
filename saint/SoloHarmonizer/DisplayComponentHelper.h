#pragma once

#include "CommonTypes.h"

#include <vector>

namespace saint {
constexpr auto widthInCrotchets = 6.f;
void setToImageWidth(const std::vector<IntervalSpan> &,
                     std::vector<IntervalSpan>::const_iterator &first,
                     std::vector<IntervalSpan>::const_iterator &last,
                     float crotchet);
} // namespace saint
