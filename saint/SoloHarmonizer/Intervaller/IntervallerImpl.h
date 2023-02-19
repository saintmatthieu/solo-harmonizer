#pragma once

#include "Intervaller.h"

#include <memory>

namespace saint {
class HarmoPitchGetter;

class IntervallerImpl : public Intervaller {
public:
  IntervallerImpl(std::unique_ptr<HarmoPitchGetter>, int ticksPerCrotchet);
  std::optional<float> getSemitoneInterval(double timeInCrotchets) override;

private:
  const std::unique_ptr<HarmoPitchGetter> _harmoPitchGetter;
  const int _ticksPerCrotchet;
};
} // namespace saint