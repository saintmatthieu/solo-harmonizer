#include "MelodyRecognizer2.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>

namespace saint {

using Melody = MelodyRecognizer2::Melody;
namespace {
float getVariance(const std::vector<float> &x) {
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
  std::vector<float> lessRefNoteNumbers;
  for (auto e = 0u; e < experiments.size(); ++e) {
    for (auto observation : experiments[e]) {
      lessRefNoteNumbers.push_back(observation - noteNumberSequence[e]);
    }
  }
  const auto variance = getVariance(lessRefNoteNumbers);
  // For now a Gauss in its most rudimentary form
  return std::expf(-variance);
}

float getDurationLikelihood(const std::vector<float> &inputLogDurations,
                            const std::vector<float> &experimentLogDurations) {
  assert(inputLogDurations.size() == experimentLogDurations.size());
  const auto N = experimentLogDurations.size();
  std::vector<float> timeRatios(N);
  for (auto i = 0u; i < N; ++i) {
    timeRatios[i] = experimentLogDurations[i] - inputLogDurations[i];
  }
  const auto variance = getVariance(timeRatios);
  return std::expf(-variance);
}

std::vector<int> getNoteNumbers(const Melody &melody) {
  std::vector<int> out(melody.size());
  std::transform(
      melody.begin(), melody.end(), out.begin(),
      [](const std::pair<float, int> &entry) { return entry.second; });
  return out;
}

std::vector<float> getLogDurations(const Melody &melody) {
  std::vector<float> out(melody.size());
  std::transform(melody.begin(), melody.end(), out.begin(),
                 [](const std::pair<float, int> &entry) {
                   return std::logf(entry.first);
                 });
  return out;
}

constexpr auto numConsideredExperiments = 2u;

std::vector<std::pair<Melody, std::vector<size_t>>>
getMotiveBeginIndices(const Melody &melody) {
  std::map<Melody, std::vector<size_t>> beginIndices;
  for (auto i = 0u; i < melody.size() - numConsideredExperiments + 1u; ++i) {
    std::vector<std::pair<float, int>> shifted(numConsideredExperiments);
    const auto firstDuration = melody[i].first;
    const auto firstPitch = melody[i].second;
    std::transform(
        melody.begin() + i, melody.begin() + i + numConsideredExperiments,
        shifted.begin(),
        [&](const std::pair<float, int> &note) -> std::pair<float, int> {
          return {note.first / firstDuration, note.second - firstPitch};
        });
    beginIndices[shifted].push_back(i);
  }
  std::vector<std::pair<Melody, std::vector<size_t>>> asVector(
      beginIndices.size());
  std::transform(beginIndices.begin(), beginIndices.end(), asVector.begin(),
                 [](const auto &entry) { return entry; });
  return asVector;
}
} // namespace

MelodyRecognizer2::MelodyRecognizer2(const Melody &melody)
    : _motiveBeginIndices(getMotiveBeginIndices(melody)) {}

std::optional<size_t>
MelodyRecognizer2::onNoteOff(const std::vector<float> &noteNumbers) {
  _lastExperiments.push_back(noteNumbers);
  if (_lastExperiments.size() < numConsideredExperiments) {
    return std::nullopt;
  } else if (_lastExperiments.size() > numConsideredExperiments) {
    _lastExperiments.erase(_lastExperiments.begin());
  }
  std::vector<float> lastExperimentLogDurations(_lastExperiments.size());
  std::transform(_lastExperiments.begin(), _lastExperiments.end(),
                 lastExperimentLogDurations.begin(),
                 [](const std::vector<float> &experiment) {
                   return std::logf(static_cast<float>(experiment.size()));
                 });
  std::vector<float> probs(_motiveBeginIndices.size());
  std::transform(_motiveBeginIndices.begin(), _motiveBeginIndices.end(),
                 probs.begin(), [&](const auto &entry) {
                   const auto &motive = entry.first;
                   const auto motiveNoteNumbers = getNoteNumbers(motive);
                   const auto motiveDurations = getLogDurations(motive);
                   const auto intervalLlh = getIntervalLikelihood(
                       motiveNoteNumbers, _lastExperiments);
                   const auto durationLlh = getDurationLikelihood(
                       motiveDurations, lastExperimentLogDurations);
                   return intervalLlh * durationLlh;
                 });
  const auto maxProbIt = std::max_element(probs.begin(), probs.end());
  const auto mostLikelyMotiveIndex = std::distance(probs.begin(), maxProbIt);
  const auto &candidateIndices =
      (_motiveBeginIndices.begin() + mostLikelyMotiveIndex)->second;
  if (!_lastGuess.has_value()) {
    _lastGuess = *candidateIndices.begin();
  } else {
    const auto nextToLastGuessIt = std::upper_bound(
        candidateIndices.begin(), candidateIndices.end(), *_lastGuess);
    if (nextToLastGuessIt != candidateIndices.end()) {
      _lastGuess = *nextToLastGuessIt;
    } else {
      _lastGuess = *std::prev(nextToLastGuessIt);
    }
  }
  return *_lastGuess + numConsideredExperiments - 1u;
}
} // namespace saint