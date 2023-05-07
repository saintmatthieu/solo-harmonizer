#pragma once

#include "MelodyRecognizer2/MelodyRecognizer2.h"
#include "MelodyTracker/MelodyRecognizer/MelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer2/MelodyRecognizer2.h"
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
                       std::unique_ptr<TimingEstimator>,
                       std::unique_ptr<MelodyRecognizer2>);
  void onHostTimeJump(float newTime) override;
  void onNoteOnSample(const std::chrono::milliseconds &when,
                      float noteNum) override;
  std::optional<size_t> onNoteOff() override;

private:
  const std::unique_ptr<MelodyRecognizer> _melodyRecognizer;
  const std::unique_ptr<TimingEstimator> _timingEstimator;
  const std::unique_ptr<MelodyRecognizer2> _melodyRecognizer2;
  std::optional<size_t> _index = 0;
  std::vector<std::pair<std::chrono::milliseconds, float>> _samples;
  std::vector<float> _observations;
};
} // namespace saint