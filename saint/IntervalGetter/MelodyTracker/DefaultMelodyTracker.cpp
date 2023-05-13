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
#include <fstream>

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

void DefaultMelodyTracker::onHostTimeJump(float newTime) {}

void DefaultMelodyTracker::onNoteOnSample(const std::chrono::milliseconds &now,
                                          float noteNum) {
  _samples.emplace_back(now, noteNum);
  _observations.emplace_back(noteNum);
}

std::optional<size_t> DefaultMelodyTracker::onNoteOff() {
  auto ret = _melodyRecognizer2->onNoteOff(_observations);
  static std::ofstream log("C:/Users/saint/downloads/log.txt");
  log << (ret.has_value() ? std::to_string(*ret) : "nullopt") << std::endl;
  if (ret.has_value()) {
    // _melodyRecognizer2 guesses what note was just played, while our client
    // expects the next note index.
    ret = *ret + 1u;
  }
  _observations.clear();
  return ret;
  // assert(!_samples.empty());
  // if (_samples.empty()) {
  //   return std::nullopt;
  // }
  // const auto fittingReady = _melodyRecognizer->onNoteOff(_samples);
  // const auto noteonSampleTime = _samples[0].first;
  // _samples.clear();
  // if (fittingReady) {
  //   const auto next = _melodyRecognizer->getNextNoteIndex();
  //   _timingEstimator->addAttack(noteonSampleTime, next - 1u);
  //   return next;
  // } else {
  //   return std::nullopt;
  // }
}
} // namespace saint