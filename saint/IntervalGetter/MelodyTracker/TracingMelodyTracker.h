#pragma once

#include "MelodyTracker.h"

#include <fstream>

namespace saint {
class TracingMelodyTracker : public MelodyTracker {
public:
  TracingMelodyTracker(std::unique_ptr<MelodyTracker> impl);
  void onHostTimeJump(float newTime) override;
  size_t onNoteOnSample(const std::chrono::milliseconds &now,
                        float noteNum) override;
  void onNoteOff() override;

private:
  const std::unique_ptr<MelodyTracker> _impl;
  std::ofstream _trace;
};
} // namespace saint