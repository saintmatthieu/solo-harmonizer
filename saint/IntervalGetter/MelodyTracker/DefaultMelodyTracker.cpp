#include "DefaultMelodyTracker.h"
#include "MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer/MelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer3/MelodyRecognizer3.h"
#include "MelodyTracker/TimingEstimator/DefaultTimingEstimator.h"
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

MelodyRecognizer3::Melody toMelodyRecognizer3Melody(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  MelodyRecognizer3::Melody out;
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
  return std::make_unique<DefaultMelodyTracker>(
      MelodyRecognizer::createInstance(melody),
      std::make_unique<DefaultTimingEstimator>(getOnsetTimes(melody)),
      std::make_unique<MelodyRecognizer3>(melody));
}

DefaultMelodyTracker::DefaultMelodyTracker(
    std::unique_ptr<MelodyRecognizer> melodyFollower,
    std::unique_ptr<TimingEstimator> timingEstimator,
    std::unique_ptr<MelodyRecognizer3> melodyRecognizer3)
    : _melodyRecognizer(std::move(melodyFollower)),
      _timingEstimator(std::move(timingEstimator)),
      _melodyRecognizer3(std::move(melodyRecognizer3)) {}

std::optional<size_t>
DefaultMelodyTracker::tick(const std::optional<float> &measuredNoteNumber,
                           float pitchConfidence) {
  static std::ofstream log("C:/Users/saint/Downloads/log.txt");
  const auto result =
      _melodyRecognizer3->tick(measuredNoteNumber, pitchConfidence);
  log << (result.has_value() ? std::to_string(*result) : "none") << std::endl;
  return result;
}

} // namespace saint