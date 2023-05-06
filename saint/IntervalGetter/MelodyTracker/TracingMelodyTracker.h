#pragma once

#include "MelodyTracker.h"

#include <fstream>

namespace saint {
class TracingMelodyTracker : public MelodyTracker {
public:
  TracingMelodyTracker(std::unique_ptr<MelodyTracker> impl);
  void onHostTimeJump(float newTime) override;
  void onNoteOnSample(const std::chrono::milliseconds &now,
                      float noteNum) override;
  std::optional<size_t> onNoteOff() override;

private:
  const std::unique_ptr<MelodyTracker> _impl;
  std::ofstream _trace;
};
} // namespace saint