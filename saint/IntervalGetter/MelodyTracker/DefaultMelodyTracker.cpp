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

void DefaultMelodyTracker::onNoteOnSample(const std::chrono::milliseconds &now,
                                          float noteNum) {
  _samples.emplace_back(now, noteNum);
}

std::optional<size_t> DefaultMelodyTracker::onNoteOff() {
  assert(!_samples.empty());
  if (_samples.empty()) {
    return std::nullopt;
  }
  const auto fittingReady = _melodyRecognizer->onNoteOff(_samples);
  const auto noteonSampleTime = _samples[0].first;
  _samples.clear();
  if (fittingReady) {
    const auto next = _melodyRecognizer->getNextNoteIndex();
    _timingEstimator->addAttack(noteonSampleTime, next - 1u);
    return next;
  } else {
    return std::nullopt;
  }
}
} // namespace saint