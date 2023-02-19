#pragma once

#include "Intervaller.h"

#include <memory>

namespace saint {
class HarmoPitchGetter;

class IntervallerImpl : public Intervaller {
public:
  IntervallerImpl(int ticksPerCrotchet, float crotchetsPerSecond,
                  std::unique_ptr<HarmoPitchGetter>);
  std::optional<float> getSemitoneInterval(int tick) override;
  int getTicksPerCrotchet() const override;
  float getCrotchetsPerSecond() const override;

private:
  const int _ticksPerCrotchet;
  const float _crotchetsPerSecond;
  const std::unique_ptr<HarmoPitchGetter> _harmoPitchGetter;
};
} // namespace saint