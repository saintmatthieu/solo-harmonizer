#include "DefaultMelodyTracker.h"
#include "MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer/MelodyRecognizer.h"
#include "MelodyTracker/TimingEstimator/DefaultTimingEstimator.h"
#include "TracingMelodyTracker.h"
#include "Utils.h"

#include <cassert>
#include <chrono>
#include <cmath>

namespace saint {
namespace {
std::vector<float>
getOnsetTimes(const std::vector<std::pair<float, std::optional<int>>> &melody) {
  std::vector<float> times;
  for (const auto &entry : melody) {
    if (entry.second.has_value()) {
      times.push_back(entry.first);
    }
  }
  return times;
}
} // namespace

std::unique_ptr<MelodyTracker> MelodyTracker::createInstance(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  auto tracker = std::make_unique<DefaultMelodyTracker>(
      MelodyRecognizer::createInstance(melody),
      std::make_unique<DefaultTimingEstimator>(getOnsetTimes(melody)));
  if (utils::getEnvironmentVariableAsBool("SAINT_DEBUG_MELODYTRACKER") &&
      utils::isDebugBuild()) {
    return std::make_unique<TracingMelodyTracker>(std::move(tracker));
  } else {
    return tracker;
  }
}

DefaultMelodyTracker::DefaultMelodyTracker(
    std::unique_ptr<MelodyRecognizer> melodyFollower,
    std::unique_ptr<TimingEstimator> timingEstimator)
    : _melodyRecognizer(std::move(melodyFollower)),
      _timingEstimator(std::move(timingEstimator)) {}

void DefaultMelodyTracker::onHostTimeJump(float newTime) {}

size_t
DefaultMelodyTracker::onNoteOnSample(const std::chrono::milliseconds &now,
                                     float noteNum) {
  _samples.emplace_back(now, noteNum);
  auto index = 0u;
  if (_index.has_value()) {
    index = *_index;
  } else {
    const auto floatIndex = _timingEstimator->estimateNoteIndex(now);
    _index = index = static_cast<size_t>(std::roundf(floatIndex));
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

  if (lookingGood && _timingEstimator->addAttack(_samples[0].first, *_index)) {
    _index =
        fittingReady ? _melodyRecognizer->getNextNoteIndex() : *_index + 1u;
  } else {
    _index.reset();
  }
  _samples.clear();
  _fittingWasReady = fittingReady;
}
} // namespace saint