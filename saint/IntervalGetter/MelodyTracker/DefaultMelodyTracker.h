#pragma once

#include "MelodyTracker/MelodyRecognizer/MelodyRecognizer.h"
#include "MelodyTracker/MelodyTracker.h"
#include "MelodyTracker/TimingEstimator/TimingEstimator.h"
#include <chrono>
#include <memory>
#include <optional>
#include <vector>

namespace saint {

class DefaultMelodyTracker : public MelodyTracker {
public:
  DefaultMelodyTracker(std::unique_ptr<MelodyRecognizer>,
                       std::unique_ptr<TimingEstimator>);
  void onHostTimeJump(float newTime) override;
  // return value: note index, or nullopt if no current note index can be
  // assumed.
  size_t onNoteOnSample(const std::chrono::milliseconds &when,
                        float noteNum) override;
  void onNoteOff() override;

private:
  const std::unique_ptr<MelodyRecognizer> _melodyRecognizer;
  const std::unique_ptr<TimingEstimator> _timingEstimator;
  std::optional<size_t> _index = 0;
  std::vector<std::pair<std::chrono::milliseconds, float>> _samples;
  bool _fittingWasReady = false;
};
} // namespace saint