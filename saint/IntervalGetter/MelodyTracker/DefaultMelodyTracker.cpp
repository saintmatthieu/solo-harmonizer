#include "DefaultMelodyTracker.h"

#include <cassert>
#include <cmath>

namespace saint {
DefaultMelodyTracker::DefaultMelodyTracker(
    const std::vector<std::pair<float, std::optional<int>>> &melody,
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