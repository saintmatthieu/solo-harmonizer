#include "DefaultMelodyTracker.h"
#include "MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer/MelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer2/MelodyRecognizer2.h"
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

MelodyRecognizer2::Melody toMelodyRecognizer2Melody(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  MelodyRecognizer2::Melody out;
  for (auto i = 1u; i < melody.size(); ++i) {
    if (melody[i - 1u].second.has_value()) {
      const auto duration = melody[i].first - melody[i - 1u].first;
      out.emplace_back(duration, *melody[i - 1u].second);
    }
  }
  return out;
}
} // namespace

std::unique_ptr<MelodyTracker> MelodyTracker::createInstance(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  auto tracker = std::make_unique<DefaultMelodyTracker>(
      MelodyRecognizer::createInstance(melody),
      std::make_unique<DefaultTimingEstimator>(getOnsetTimes(melody)),
      std::make_unique<MelodyRecognizer2>(toMelodyRecognizer2Melody(melody)));
  if (utils::getEnvironmentVariableAsBool("SAINT_DEBUG_MELODYTRACKER") &&
      utils::isDebugBuild()) {
    return std::make_unique<TracingMelodyTracker>(std::move(tracker));
  } else {
    return tracker;
  }
}

DefaultMelodyTracker::DefaultMelodyTracker(
    std::unique_ptr<MelodyRecognizer> melodyFollower,
    std::unique_ptr<TimingEstimator> timingEstimator,
    std::unique_ptr<MelodyRecognizer2> melodyRecognizer2)
    : _melodyRecognizer(std::move(melodyFollower)),
      _timingEstimator(std::move(timingEstimator)),
      _melodyRecognizer2(std::move(melodyRecognizer2)) {}

std::optional<size_t> DefaultMelodyTracker::beginNewNote(int tick) {
  return _melodyRecognizer2->beginNewNote(tick);
}

void DefaultMelodyTracker::addPitchMeasurement(float pc) {
  _melodyRecognizer2->addPitchMeasurement(pc);
}

} // namespace saint