#include "MelodyRecognizer3.h"

#include <limits>

namespace saint {
using Melody = MelodyRecognizer3::Melody;

namespace {
constexpr auto noPitchState = std::numeric_limits<int>::min();

std::unordered_map<size_t, std::unordered_map<size_t, float>>
getTransitionTable(const Melody &melody) {
  std::unordered_map<size_t, std::unordered_map<size_t, float>> table;
  const auto totalDuration = melody[melody.size() - 1u].first - melody[0].first;
  for (auto i = 0u; i < melody.size() - 1u; ++i) {
    auto &iCol = table[i];
    const auto stayLlh =
        std::logf((melody[i + 1u].first - melody[i].first) / totalDuration);
    for (auto j = 0u; j < melody.size() - 1u; ++j) {
      if (i == j) { // Stay in the same state
        iCol[j] = stayLlh;
      } else if ()
    }
  }
  return table;
}

std::unordered_map<size_t, float> getInitialPriors(size_t numStates) {
  // For now initially just give equal chance to all states. Maybe priviledging
  // index 0, assuming that users typically begin at the start, could be an
  // option. But that certainly shouldn't be a constraint.
  const auto prior = -std::logf(static_cast<size_t>(numStates));
  std::unordered_map<size_t, float> priors;
  for (auto i = 0u; i < numStates; ++i) {
    priors[i] = prior;
  }
  return priors;
}

float getObservationLikelihood(int tryState,
                               const std::optional<float> &pitchMeasurement) {}
} // namespace

MelodyRecognizer3::MelodyRecognizer3(const Melody &melody)
    : _numStates(melody.size() - 1u),
      _transitionTable(getTransitionTable(melody)),
      _priors(getInitialPriors(_numStates)) {}

std::optional<size_t>
MelodyRecognizer3::tick(const std::optional<float> &detectedPitch) {
  std::optional<float> maxProb;
  int maxProbState = noPitchState;
  for (const auto oldState : _stateSet) {
    for (const auto newState : _stateSet) {
      // const auto prob =
      // _priors.at(oldState)+_transitionTable.at(oldState).at(newState)+
    }
  }
  return std::nullopt;
}

} // namespace saint