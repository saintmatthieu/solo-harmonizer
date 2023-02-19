#include "IntervallerImpl.h"

#include "HarmoPitchGetter.h"

namespace saint {
IntervallerImpl::IntervallerImpl(
    std::unique_ptr<HarmoPitchGetter> harmoPitchGetter, int ticksPerCrotchet)
    : _harmoPitchGetter(std::move(harmoPitchGetter)),
      _ticksPerCrotchet(ticksPerCrotchet) {}

std::optional<float>
IntervallerImpl::getSemitoneInterval(double timeInCrotchets) {
  return _harmoPitchGetter->getHarmoInterval(_ticksPerCrotchet *
                                             timeInCrotchets);
}
} // namespace saint
