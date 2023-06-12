#include "MelodyRecognizer3.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

namespace saint {
using Melody = MelodyRecognizer3::Melody;

namespace {
constexpr auto noPitchState = std::numeric_limits<int>::min();

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

float getObservationLikelihood(int hypothesisNoteNumber,
                               float measuredNoteNumber) {
  // A PDF does not read as probabilities, but I think we're only interested
  // in the relative values, so let's just do this. The log of a normal
  // distribution is is proportional to the square of the error. Again, we're
  // just interested in how the probabilities compare, so we ignore all
  // scalars and just use
  const auto error = hypothesisNoteNumber - measuredNoteNumber;
  return -error * error;
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
} // namespace

MelodyRecognizer3::MelodyRecognizer3(Melody melody)
    : _melody(std::move(melody)),
      _stateToMelodyIndices(getMelodyNoteIndices(_melody)),
      _priors(getInitialPriors(_stateToMelodyIndices.size())),
      _newPriors(_priors.size()) {}

std::optional<size_t>
MelodyRecognizer3::tick(const std::optional<float> &measuredNoteNumber) {
  _stateCount = measuredNoteNumber.has_value() ? _stateCount + 1u : 0u;
  if (!measuredNoteNumber.has_value()) {
    ++_tickCount;
    return std::nullopt;
  }
  std::optional<float> maxProb;
  int maxProbState = noPitchState;
  const auto observationLlhs = _getObservationLikelihoods(*measuredNoteNumber);
  std::vector<size_t> newPriorAntecedentIndices(_stateToMelodyIndices.size());
  for (auto newState = 0u; newState < _stateToMelodyIndices.size();
       ++newState) {
    auto maxProb = -std::numeric_limits<float>::max();
    auto maxProbIndex = 0u;
    for (auto oldState = 0u; oldState < _stateToMelodyIndices.size();
         ++oldState) {
      const auto transitionLikelihood =
          _getTransitionLikelihood(oldState, newState);
      const auto prob = _priors[oldState] + std::logf(transitionLikelihood) +
                        observationLlhs[newState];
      if (prob > maxProb) {
        maxProb = prob;
        maxProbIndex = oldState;
      }
    }
    _newPriors[newState] = maxProb;
    newPriorAntecedentIndices[newState] = maxProbIndex;
  }
  const auto winnerIndex =
      std::distance(_newPriors.begin(),
                    std::max_element(_newPriors.begin(), _newPriors.end()));
  std::transform(_newPriors.begin(), _newPriors.end(), _priors.begin(),
                 [&](float prob) { return prob - _newPriors[winnerIndex]; });
  static auto prevIndex = -1;
  if (static_cast<int>(winnerIndex) != prevIndex) {
    static std::ofstream labels("C:/Users/saint/Downloads/labels.txt");
    const auto time =
        static_cast<float>(_tickCount) * samplesPerBlock / 44100.f;
    labels << time << "\t" << time << "\t" << _stateToMelodyIndices[winnerIndex]
           << std::endl;
    prevIndex = winnerIndex;
  }
  ++_tickCount;
  return _stateToMelodyIndices[winnerIndex] +
         1u; /*this +1 might be due to some integration incorrectness - to
               review*/
}

float MelodyRecognizer3::_getTransitionLikelihood(size_t oldState,
                                                  size_t newState) const {
  const auto N = _stateToMelodyIndices.size();
  const auto numCrotchets =
      _melody[_stateToMelodyIndices[oldState] + 1u].first -
      _melody[_stateToMelodyIndices[oldState]].first;
  const auto numBlocksExpectedInOldState = std::max<float>(
      0.f, blocksPerCrotchet * numCrotchets - static_cast<float>(_stateCount));
  // If there is a transition from one index to the other, the likelihood that
  // it is from an index i to i+1.
  constexpr auto transitionsToNextLlh = .9f;
  const auto llhThatItChanges =
      1.f / (numBlocksExpectedInOldState + 1.f +
             1.f // Another one to clip llhThatItChanges to 0.5 ; I don't think
                 // it should ever get much higher, and certainly not 1.
            );
  const auto llhThatItStays = 1.f - llhThatItChanges;
  if (oldState == newState) { // Stay in the same state
    return llhThatItStays;
  } else if (newState == oldState + 1u) {
    return (1.f - llhThatItStays) * transitionsToNextLlh / N;
  } else {
    return (1.f - llhThatItStays) * (1.f - transitionsToNextLlh) / N;
  }
}

std::vector<float>
MelodyRecognizer3::_getObservationLikelihoods(float measuredNoteNumber) const {
  std::vector<float> llhs(_stateToMelodyIndices.size());
  for (auto i = 0u; i < _stateToMelodyIndices.size(); ++i) {
    const auto noteIndex = _stateToMelodyIndices[i];
    llhs[i] = getObservationLikelihood(*_melody[noteIndex].second,
                                       measuredNoteNumber);
  }
  return llhs;
}

} // namespace saint