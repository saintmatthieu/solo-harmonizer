#include "DefaultMelodyTracker.h"
#include "MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer/MelodyRecognizer.h"
#include "MelodyTracker/TimingEstimator/DefaultTimingEstimator.h"

#include <__msvc_chrono.hpp>
#include <cassert>
#include <chrono>
#include <cmath>

namespace saint {
class DefaultClock : public Clock {
public:
  DefaultClock() : _creationTime(std::chrono::steady_clock::now()) {}

  std::chrono::milliseconds now() const override {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - _creationTime);
  }

private:
  const std::chrono::steady_clock::time_point _creationTime;
};

std::unique_ptr<MelodyTracker> MelodyTracker::createInstance(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  return std::make_unique<DefaultMelodyTracker>(
      MelodyRecognizer::createInstance(melody),
      std::make_unique<DefaultTimingEstimator>(),
      std::make_unique<DefaultClock>());
}

DefaultMelodyTracker::DefaultMelodyTracker(
    std::unique_ptr<MelodyRecognizer> melodyFollower,
    std::unique_ptr<TimingEstimator> timingEstimator,
    std::unique_ptr<Clock> clock)
    : _melodyRecognizer(std::move(melodyFollower)),
      _timingEstimator(std::move(timingEstimator)), _clock(std::move(clock)) {}

void DefaultMelodyTracker::onHostTimeJump(float newTime) {}

size_t DefaultMelodyTracker::onNoteOnSample(float noteNum) {
  const auto now = _clock->now();
  _samples.emplace_back(now, noteNum);
  auto index = 0u;
  if (_index.has_value()) {
    index = *_index;
  } else {
    const auto floatIndex = _timingEstimator->estimateNoteIndex(now);
    index = static_cast<size_t>(std::roundf(floatIndex));
    _index = index;
  }
  return index;
}

void DefaultMelodyTracker::onNoteOff() {

  assert(!_samples.empty());
  if (_samples.empty()) {
    return;
  }

  const auto fittingReady = _melodyRecognizer->onNoteOff(_samples);
  const auto lookingGood =
      _index.has_value() && (fittingReady || !_fittingWasReady);

  if (lookingGood) {
    _timingEstimator->addAttack(_samples[0].first, *_index);
    _index =
        fittingReady ? _melodyRecognizer->getNextNoteIndex() : *_index + 1u;
  } else {
    _index.reset();
  }

  _samples.clear();
  _fittingWasReady = fittingReady;
}
} // namespace saint