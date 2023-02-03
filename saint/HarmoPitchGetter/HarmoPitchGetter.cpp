#include "HarmoPitchGetter.h"

namespace saint {

HarmoPitchGetter::HarmoPitchGetter(
    const std::vector<InputTimeEntry> &timeSegments) {}

std::optional<float> HarmoPitchGetter::getHarmoPitch(int tick,
                                                     float semisFromA4) const {
  return std::nullopt;
}

} // namespace saint