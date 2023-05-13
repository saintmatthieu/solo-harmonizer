#include "MelodyRecognizer2.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <numeric>

namespace saint {

using Melody = MelodyRecognizer2::Melody;
using MotiveInstance = MelodyRecognizer2::MotiveInstance;

namespace {
float getAverage(const std::vector<float> &x) {
  const auto N = static_cast<float>(x.size());
  return std::accumulate(x.begin(), x.end(), 0.f) / N;
}

float getVariance(const std::vector<float> &x, float avg) {
  return std::accumulate(x.begin(), x.end(), 0.f,
                         [avg](float acc, float x) {
                           return acc + (x - avg) * (x - avg);
                         }) /
         static_cast<float>(x.size());
}

std::pair<float /*avg*/, float /*likelihood*/>
getIntervalLikelihood(const std::vector<int> &noteNumberSequence,
                      const std::vector<std::vector<float>> &experiments) {
  assert(experiments.size() == noteNumberSequence.size());
  std::vector<float> lessRefNoteNumbers;
  for (auto e = 0u; e < experiments.size(); ++e) {
    for (auto observation : experiments[e]) {
      lessRefNoteNumbers.push_back(observation - noteNumberSequence[e]);
    }
  }
  const auto avg = getAverage(lessRefNoteNumbers);
  const auto variance = getVariance(lessRefNoteNumbers, avg);
  // Variance here has unit semitone squared. E.g.
  //    Reference pitch : [60, 60]
  //    Actual pitch : [63, 65]
  //    Fit : [59, 61] => variance = 1
  // We penalize this giving a probability of 1/2.
  return {avg, std::powf(2.f, -variance)};
}

std::pair<float /*avg*/, float /*likelihood*/>
getDurationLikelihood(const std::vector<float> &inputLogDurations,
                      const std::vector<float> &experimentLogDurations) {
  assert(inputLogDurations.size() == experimentLogDurations.size());
  const auto N = experimentLogDurations.size();
  std::vector<float> timeRatios(N);
  for (auto i = 0u; i < N; ++i) {
    timeRatios[i] = experimentLogDurations[i] - inputLogDurations[i];
  }
  const auto avg = getAverage(timeRatios);
  const auto variance = getVariance(timeRatios, avg);
  // Variance here has unit "time octave" squared. E.g.
  //    Reference duration : [1, 1] (sec) -> log -> [0, 0]
  //    Actual duration : [2, 8] (sec) -> log -> [1, 3]
  //    Fit (log) : [-1, 1] => variance = 1.
  // We penalize this giving a probability of 1/2. It may not seem much, but we
  // should probably be less severe with duration as with pitch, people don't
  // necessarily care about muting in time.
  return {avg, std::powf(2.f, -variance)};
}

std::vector<int> getNoteNumbers(const Melody &melody) {
  std::vector<int> out(melody.size());
  std::transform(
      melody.begin(), melody.end(), out.begin(),
      [](const std::pair<float, int> &entry) { return entry.second; });
  return out;
}

std::vector<float> getDurations(const Melody &melody) {
  std::vector<float> out(melody.size());
  std::transform(
      melody.begin(), melody.end(), out.begin(),
      [](const std::pair<float, int> &entry) { return entry.first; });
  return out;
}

constexpr auto numConsideredExperiments = 2u;

std::vector<std::pair<Melody, std::vector<MotiveInstance>>>
getMotiveBeginInstances(const Melody &melody) {
  std::map<Melody, std::vector<MotiveInstance>> motiveInstances;
  for (auto i = 0u; i < melody.size() - numConsideredExperiments + 1u; ++i) {
    std::vector<std::pair<float, int>> shifted(numConsideredExperiments);
    const auto firstDuration = melody[i].first;
    const auto firstPitch = melody[i].second;
    std::transform(
        melody.begin() + i, melody.begin() + i + numConsideredExperiments,
        shifted.begin(),
        [&](const std::pair<float, int> &note) -> std::pair<float, int> {
          return {note.first - firstDuration, note.second - firstPitch};
        });
    motiveInstances[shifted].push_back({i, firstPitch, firstDuration});
  }
  std::vector<std::pair<Melody, std::vector<MotiveInstance>>> asVector(
      motiveInstances.size());
  std::transform(motiveInstances.begin(), motiveInstances.end(),
                 asVector.begin(), [](const auto &entry) { return entry; });
  return asVector;
}

float getReferenceDuration(const Melody &melody) {
  // A pitch class p is a log2(f/f0) operation. For our purpose it is convenient
  // to have durations in similar form. As duration reference we choose the
  // least duration. Ideally this is quantized and we can get nice round values
  // when applying our log2 formula.
  return std::log2f(std::min_element(melody.begin(), melody.end(),
                                     [](const auto &a, const auto &b) {
                                       return a.first < b.first;
                                     })
                        ->first);
}

Melody convertDurationsToLog(Melody melody, float referenceDuration) {
  std::transform(
      melody.begin(), melody.end(), melody.begin(),
      [referenceDuration](const auto &entry) -> std::pair<float, int> {
        return {std::log2f(entry.first) - referenceDuration, entry.second};
      });
  return melody;
}
} // namespace

MelodyRecognizer2::MelodyRecognizer2(Melody melody)
    : _referenceDuration(getReferenceDuration(melody)),
      _motiveInstances(getMotiveBeginInstances(
          convertDurationsToLog(std::move(melody), _referenceDuration))) {}

std::optional<size_t>
MelodyRecognizer2::onNoteOff(const std::vector<float> &noteNumbers) {
  static std::ofstream log("C:/Users/saint/downloads/log.txt");

  _lastExperiments.push_back(noteNumbers);
  if (_lastExperiments.size() < numConsideredExperiments) {
    log << "nullopt" << std::endl;
    return std::nullopt;
  } else if (_lastExperiments.size() > numConsideredExperiments) {
    _lastExperiments.erase(_lastExperiments.begin());
  }
  std::vector<float> lastExperimentLogDurations(_lastExperiments.size());
  std::transform(_lastExperiments.begin(), _lastExperiments.end(),
                 lastExperimentLogDurations.begin(),
                 [this](const std::vector<float> &experiment) {
                   // We could try to convert observation sample rate time unit
                   // to that of the reference duration to get nice round
                   // numbers. Would simplify debugging, but it's correct as it
                   // is.
                   return std::log2f(static_cast<float>(experiment.size()) -
                                     _referenceDuration);
                 });
  struct Stats {
    float durationErrorAvg;
    float pitchClassErrorAvg;
    float combinedLikelihood;
  };
  std::vector<Stats> stats(_motiveInstances.size());
  std::transform(
      _motiveInstances.begin(), _motiveInstances.end(), stats.begin(),
      [&](const auto &entry) {
        const auto &motive = entry.first;
        const auto motiveNoteNumbers = getNoteNumbers(motive);
        const auto motiveDurations = getDurations(motive);
        const auto [pitchClassErrorAvg, intervalLlh] =
            getIntervalLikelihood(motiveNoteNumbers, _lastExperiments);
        const auto [durationErrorAvg, durationLlh] =
            getDurationLikelihood(motiveDurations, lastExperimentLogDurations);
        return Stats{durationErrorAvg, pitchClassErrorAvg,
                     intervalLlh * durationLlh};
      });
  const auto maxProbIt = std::max_element(
      stats.begin(), stats.end(), [](const Stats &a, const Stats &b) {
        return a.combinedLikelihood < b.combinedLikelihood;
      });
  if (maxProbIt->combinedLikelihood < 0.25f) {
    // For perfect timing this means a variance between 1 and 2 semitones (not
    // sure exactly which value at this time.)
    log << "nullopt" << std::endl;
    return std::nullopt;
  }
  const auto mostLikelyMotiveIndex = std::distance(stats.begin(), maxProbIt);
  const auto &candidateInstances =
      (_motiveInstances.begin() + mostLikelyMotiveIndex)->second;
  std::vector<size_t> candidateIndices(candidateInstances.size());
  std::transform(candidateInstances.begin(), candidateInstances.end(),
                 candidateIndices.begin(), [](const MotiveInstance &instance) {
                   return instance.beginIndex;
                 });
  auto indexOfChosenCandidate = 0u;
  if (_lastGuess.has_value()) {
    const auto nextToLastGuessIt = std::upper_bound(
        candidateIndices.begin(), candidateIndices.end(), *_lastGuess);
    if (nextToLastGuessIt != candidateIndices.end()) {
      indexOfChosenCandidate =
          std::distance(candidateIndices.begin(), nextToLastGuessIt);
    } else {
      indexOfChosenCandidate =
          std::distance(candidateIndices.begin(), std::prev(nextToLastGuessIt));
    }
  }
  _lastGuess = candidateIndices[indexOfChosenCandidate];
  const auto pitchTransposition =
      maxProbIt->pitchClassErrorAvg -
      candidateInstances[indexOfChosenCandidate].firstNoteNumber;
  const auto durationTransposition =
      maxProbIt->durationErrorAvg -
      candidateInstances[indexOfChosenCandidate].firstDuration;
  const auto retval = *_lastGuess + numConsideredExperiments - 1u;
  log << "index=" << retval << ", pitchTranspose: " << pitchTransposition
      << ", durationTranspose: " << durationTransposition << std::endl;
  return retval;
}
} // namespace saint