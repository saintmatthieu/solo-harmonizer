#include "MelodyRecognizer3.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

namespace saint {
using Melody = MelodyRecognizer3::Melody;

namespace {
constexpr auto noPitchState = std::numeric_limits<int>::min();

constexpr auto bestLlhThatItStays = 1.f;
constexpr auto bestTransitionToNextLlh = 1.f;
constexpr auto bestObservationLlhWeight = 1.f;

// If useful, this information will be passed to the ctor. But this is still
// experimentation, so for now just assuming these values from the unit test
// used to debug this code.
constexpr auto samplesPerBlock = 512.f;
constexpr auto samplesPerCrotchet = 22000;
constexpr auto blocksPerCrotchet = samplesPerCrotchet / samplesPerBlock;

std::vector<float> getInitialPriors(size_t numStates) {
  // Say there's a relatively higher chance that the user begins at the
  // beginning.
  constexpr auto endStateWeight = 2.f;
  constexpr auto otherWeights = 1.f;
  const auto weightSum = endStateWeight + (numStates - 1) * otherWeights;
  const auto endStatePrior = std::logf(endStateWeight / weightSum);
  const auto otherPriors = std::logf(otherWeights / weightSum);
  std::vector<float> priors(numStates);
  priors[0u] = endStatePrior;
  for (auto i = 1u; i < numStates; ++i) {
    priors[i] = otherPriors;
  }
  return priors;
}

float getObservationLikelihood(
    int hypothesisNoteNumber,
    const std::function<float(int)> &getPitchLikelihood,
    float observationLlhWeight) {
  // If observationLlhWeight is high, we are more confident in the observation.
  // Unfortunately, we have to expf to logf again if we want to take this into
  // account. Might be optimized otherwise, but not now.
  return std::logf(observationLlhWeight *
                       getPitchLikelihood(hypothesisNoteNumber) +
                   1.f - observationLlhWeight);
}

std::vector<size_t> getMelodyNoteIndices(const Melody &melody) {
  std::vector<size_t> noteIndices;
  for (auto i = 0u; i < melody.size(); ++i) {
    const auto &noteNumber = melody[i].second;
    if (noteNumber.has_value()) {
      noteIndices.push_back(i);
    }
  }
  return noteIndices;
}

std::string getTime(size_t tickCount) {
  return std::to_string(static_cast<float>(tickCount) * samplesPerBlock /
                        44100.f);
}

void printNullopt(std::ofstream &labels, size_t tickCount,
                  bool prevReturnHadValue) {
  if (prevReturnHadValue) {
    const auto time = getTime(tickCount);
    labels << time << "\t" << time << "\t" << tickCount << ": null return"
           << std::endl;
  }
}

std::vector<std::vector<float>>
getTransitionMatrix(const Melody &melody,
                    const std::vector<size_t> &stateToMelodyNoteIndices) {
  const auto N = stateToMelodyNoteIndices.size();
  std::vector<std::vector<float>> matrix;
  matrix.reserve(N);
  for (auto i = 0u; i < N; ++i) {
    matrix.emplace_back(std::vector<float>(N));
  }
  const auto totalDuration = melody[melody.size() - 1u].first - melody[0].first;
  // If there is a transition from one index to the other, the likelihood that
  // it is from an index i to i+1.
  const auto transitionsToNextLlh = .95f;
  for (auto oldState = 0u; oldState < N; ++oldState) {
    const auto i = stateToMelodyNoteIndices[oldState];
    const auto j = oldState < N - 1 ? stateToMelodyNoteIndices[oldState + 1]
                                    // because `melody` ends with nullopt
                                    : i + 1;
    const auto numCrotchets = melody[j].first - melody[i].first;
    // Likelihood that we stay at the same index.
    const auto numBlocksExpectedInOldState = blocksPerCrotchet * numCrotchets;
    const auto llhThatItChanges = 1.f / (numBlocksExpectedInOldState + 1.f);
    const auto llhThatItStays = 1.f - llhThatItChanges;
    for (auto newState = 0u; newState < N; ++newState) {
      auto &cell = matrix[oldState][newState];
      if (oldState == newState) {
        cell = llhThatItStays;
      } else if (newState == oldState + 1u) {
        cell = (1.f - llhThatItStays) * transitionsToNextLlh;
      } else {
        cell = (1.f - llhThatItStays) * (1.f - transitionsToNextLlh);
      }
    }
  }
  return matrix;
}
} // namespace

MelodyRecognizer3::MelodyRecognizer3(
    Melody melody, std::optional<float> observationLikelihoodWeight)
    : _melody(std::move(melody)),
      _observationLikelihoodWeight(
          observationLikelihoodWeight.value_or(0.007f)),
      _stateToMelodyIndices(getMelodyNoteIndices(_melody)),
      _transitionMatrix(getTransitionMatrix(_melody, _stateToMelodyIndices)),
      _priors(getInitialPriors(_stateToMelodyIndices.size())),
      _newPriors(_priors.size()) {}

std::optional<size_t> MelodyRecognizer3::tick(
    const std::optional<std::function<float(float)>> &getPitchLikelihood,
    std::optional<size_t> &guessedState) {
  static std::ofstream output("C:/Users/saint/Downloads/output.txt");
  static std::ofstream labels("C:/Users/saint/Downloads/labels.txt");
  _stateCount = getPitchLikelihood.has_value() ? _stateCount + 1u : 0u;
  if (!getPitchLikelihood.has_value()) {
    ++_tickCount;
    output << -1 << std::endl;
    printNullopt(labels, _tickCount, _prevReturnHadValue);
    _prevReturnHadValue = false;
    return std::nullopt;
  }

  static std::unique_ptr<Params> params;
  if (!params) {
    std::ifstream ifs("C:/Users/saint/Downloads/params.txt");
    float llhThatItStays, transitionToNextLlh, observationLlhWeight;
    ifs >> llhThatItStays >> transitionToNextLlh >> observationLlhWeight;
    params.reset(
        new Params{llhThatItStays, transitionToNextLlh, observationLlhWeight});
  }

  const auto observationLlhs = _getObservationLikelihoods(*getPitchLikelihood);

  std::optional<float> maxProb;
  int maxProbState = noPitchState;
  std::vector<size_t> newPriorAntecedentIndices(_stateToMelodyIndices.size());
  for (auto newState = 0u; newState < _stateToMelodyIndices.size();
       ++newState) {
    auto maxProb = -std::numeric_limits<float>::max();
    auto maxProbIndex = 0u;
    for (auto oldState = 0u; oldState < _stateToMelodyIndices.size();
         ++oldState) {
      const auto transitionLikelihood =
          // The transition matrix isn't pre-computed in log form to more easily
          // verify that its colums sum up to 1 while debugging. (Still not sure
          // this is a must for the use we're making of it, though.)
          std::logf(_transitionMatrix[oldState][newState]);
      const auto prob =
          _priors[oldState] + transitionLikelihood + observationLlhs[newState];
      if (prob > maxProb) {
        maxProb = prob;
        maxProbIndex = oldState;
      }
    }
    _newPriors[newState] = maxProb;
    newPriorAntecedentIndices[newState] = maxProbIndex;
  }
  const auto winnerState =
      std::distance(_newPriors.begin(),
                    std::max_element(_newPriors.begin(), _newPriors.end()));
  const auto winnerOriginState = newPriorAntecedentIndices[winnerState];
  const auto melodyIndex = _stateToMelodyIndices[winnerState];
  std::transform(_newPriors.begin(), _newPriors.end(), _priors.begin(),
                 [&](float prob) { return prob - _newPriors[winnerState]; });
  static auto prevIndex = -1;
  if (static_cast<int>(winnerState) != prevIndex || !_prevReturnHadValue) {
    const auto time = getTime(_tickCount);
    const auto nn = *_melody[melodyIndex].second;
    labels << time << "\t" << time << "\t" << _tickCount
           << ": guess=" << winnerState << ", NN=" << nn
           << ", orig=" << winnerOriginState << ", stateCount=" << _stateCount
           << std::endl;
    prevIndex = winnerState;
  }
  // if (winnerOriginState != _winnerIndex) {
  //   const auto time = getTime(_tickCount);
  //   labels << time << "\t" << time << "\tChange of mind!" << std::endl;
  // }
  output << winnerState << std::endl;
  _winnerIndex = winnerState;
  ++_tickCount;
  _prevReturnHadValue = true;
  guessedState = winnerState;
  return melodyIndex + 1u; /*this +1 might be due to some integration
                             incorrectness - to review*/
}

std::vector<float> MelodyRecognizer3::_getObservationLikelihoods(
    const std::function<float(int)> &getPitchLikelihood) const {
  std::vector<float> llhs(_stateToMelodyIndices.size());
  for (auto i = 0u; i < _stateToMelodyIndices.size(); ++i) {
    const auto noteIndex = _stateToMelodyIndices[i];
    llhs[i] =
        getObservationLikelihood(*_melody[noteIndex].second, getPitchLikelihood,
                                 _observationLikelihoodWeight);
  }
  return llhs;
}

} // namespace saint