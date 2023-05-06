#include "MelodyRecognizer2.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>

namespace saint {
namespace {
float getMeanSquared(const std::vector<float> &x) {
  const auto N = static_cast<float>(x.size());
  const auto avg = std::accumulate(x.begin(), x.end(), 0.f) / N;
  return std::accumulate(x.begin(), x.end(), 0.f,
                         [avg](float acc, float x) {
                           return acc + (x - avg) * (x - avg);
                         }) /
         N;
}

float getIntervalLikelihood(
    const std::vector<int> &noteNumberSequence,
    const std::vector<std::vector<float>> &experiments) {
  assert(experiments.size() == noteNumberSequence.size());
  std::vector<float> centered;
  for (auto e = 0u; e < experiments.size(); ++e) {
    for (auto observation : experiments[e]) {
      centered.push_back(observation - noteNumberSequence[e]);
    }
  }
  const auto meanSquared = getMeanSquared(centered);
  // For now a Gauss in its most rudimentary form
  return std::expf(-meanSquared);
}

float getDurationLikelihood(const std::vector<float> &inputDurations,
                            const std::vector<size_t> &experimentDurations) {
  assert(inputDurations.size() == experimentDurations.size());
  const auto N = experimentDurations.size();
  std::vector<float> timeRatios(N);
  for (auto i = 0u; i < N; ++i) {
    timeRatios[i] =
        static_cast<float>(experimentDurations[i]) / inputDurations[i];
  }
  const auto avg = std::accumulate(timeRatios.begin(), timeRatios.end(), 0.f) /
                   static_cast<float>(N);
  std::vector<float> centered(N);
  std::transform(timeRatios.begin(), timeRatios.end(), centered.begin(),
                 [avg](float r) { return r - avg; });
  const auto meanSquared = getMeanSquared(centered);
  return std::expf(-meanSquared);
}

std::vector<int> getNoteNumbers(const MelodyRecognizer2::Melody &melody) {
  std::vector<int> out(melody.size());
  std::transform(
      melody.begin(), melody.end(), out.begin(),
      [](const std::pair<float, int> &entry) { return entry.second; });
  return out;
}

std::vector<float> getDurations(const MelodyRecognizer2::Melody &melody) {
  std::vector<float> out(melody.size());
  std::transform(
      melody.begin(), melody.end(), out.begin(),
      [](const std::pair<float, int> &entry) { return entry.first; });
  return out;
}

constexpr auto numConsideredExperiments = 2u;

} // namespace

MelodyRecognizer2::MelodyRecognizer2(Melody melody)
    : _melody(std::move(melody)), _noteNumbers(getNoteNumbers(_melody)),
      _durations(getDurations(_melody)) {}

std::optional<size_t>
MelodyRecognizer2::onNoteOff(const std::vector<float> &noteNumbers) {
  if (_melody.size() < numConsideredExperiments) {
    assert(false);
    return std::nullopt;
  }
  _lastExperiments.push_back(noteNumbers);
  if (_lastExperiments.size() < numConsideredExperiments) {
    return std::nullopt;
  } else if (_lastExperiments.size() > numConsideredExperiments) {
    _lastExperiments.erase(_lastExperiments.begin());
  }
  std::vector<size_t> _lastExperimentDurations(_lastExperiments.size());
  std::transform(
      _lastExperiments.begin(), _lastExperiments.end(),
      _lastExperimentDurations.begin(),
      [](const std::vector<float> &experiment) { return experiment.size(); });
  std::vector<float> probs(_melody.size() - numConsideredExperiments + 1u);
  for (auto i = 0u; i < probs.size(); ++i) {
    const auto intervalLlh = getIntervalLikelihood(
        {_noteNumbers.data() + i,
         _noteNumbers.data() + i + numConsideredExperiments},
        _lastExperiments);
    const auto durationLlh = getDurationLikelihood(
        {_durations.begin() + i,
         _durations.begin() + i + numConsideredExperiments},
        _lastExperimentDurations);
    probs[i] = intervalLlh * durationLlh;
  }
  const auto maxProbIt = std::max_element(probs.begin(), probs.end());
  return std::distance(probs.begin(), maxProbIt) + numConsideredExperiments -
         1u;
}
} // namespace saint