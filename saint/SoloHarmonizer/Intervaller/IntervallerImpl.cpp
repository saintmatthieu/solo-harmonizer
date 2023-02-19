#include "IntervallerImpl.h"

#include "HarmoPitchGetter.h"

namespace saint {
IntervallerImpl::IntervallerImpl(
    int ticksPerCrotchet, float crotchetsPerSecond,
    std::unique_ptr<HarmoPitchGetter> harmoPitchGetter)
    : _ticksPerCrotchet(ticksPerCrotchet),
      _crotchetsPerSecond(crotchetsPerSecond),
      _harmoPitchGetter(std::move(harmoPitchGetter)) {}

std::optional<float> IntervallerImpl::getSemitoneInterval(int tick) {
  return _harmoPitchGetter->getHarmoInterval(tick);
}

int IntervallerImpl::getTicksPerCrotchet() const { return _ticksPerCrotchet; }

float IntervallerImpl::getCrotchetsPerSecond() const {
  return _crotchetsPerSecond;
}
} // namespace saint