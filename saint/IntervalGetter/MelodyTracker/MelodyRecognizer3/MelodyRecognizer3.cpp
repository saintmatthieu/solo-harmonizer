#include "MelodyRecognizer3.h"

#include <algorithm>
#include <cmath>
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

std::vector<std::vector<float>> getTransitionMatrix(const Melody &melody) {
  const auto N = melody.size() - 1u;
  std::vector<std::vector<float>> matrix;
  matrix.reserve(N);
  for (auto i = 0u; i < N; ++i) {
    matrix.emplace_back(std::vector<float>(N));
  }
  const auto totalDuration = melody[N].first - melody[0].first;
  // If there is a transition from one index to the other, the likelihood that
  // it is from an index i to i+1.
  const auto transitionsToNextLlh = .9f;
  for (auto oldState = 0u; oldState < N; ++oldState) {
    const auto numCrotchets =
        melody[oldState + 1u].first - melody[oldState].first;
    // Likelihood that we stay at the same index.
    const auto numBlocksExpectedInThisState = blocksPerCrotchet * numCrotchets;
    const auto llhThatItChanges = 1.f / (numBlocksExpectedInThisState + 1.f);
    const auto llhThatItStays = 1.f - llhThatItChanges;
    for (auto newState = 0u; newState < N; ++newState) {
      auto &cell = matrix[newState][oldState]; // Yes, the other way round.
      if (oldState == newState) {              // Stay in the same state
        cell = std::logf(llhThatItStays);
      } else if (newState == oldState + 1u) {
        cell = std::logf((1.f - llhThatItStays) * transitionsToNextLlh);
      } else {
        cell = std::logf((1.f - llhThatItStays) * (1.f - transitionsToNextLlh));
      }
    }
  }
  return matrix;
}

std::vector<float> getInitialPriors(size_t numStates) {
  // Say there's a relatively higher chance that the user begins at the
  // beginning.
  constexpr auto zeroWeight = 2.f;
  constexpr auto otherWeights = 1.f;
  const auto weightSum = zeroWeight + (numStates - 1) * otherWeights;
  const auto firstPrior = std::logf(zeroWeight / weightSum);
  const auto otherPriors = std::logf(otherWeights / weightSum);
  std::vector<float> priors(numStates);
  priors[0u] = firstPrior;
  for (auto i = 1u; i < numStates; ++i) {
    priors[i] = otherPriors;
  }
  return priors;
}

float getObservationLikelihood(const std::optional<int> &hypothesisNoteNumber,
                               const std::optional<float> &measuredNoteNumber) {
  if (!hypothesisNoteNumber.has_value() && !measuredNoteNumber.has_value()) {
    return 0.f;
  } else if (hypothesisNoteNumber.has_value() &&
             !measuredNoteNumber.has_value()) {
    // Log likelihood that no pitch is detected although there the hidden state
    // is a note number. This could be the case if the played note ends earlier
    // than in the score (e.g. poor sustain guitar or difficulty playing
    // legato), or, of course, if the player is a bit late playing the note. So
    // don't be too harsh. Penalize this like a an offset of a quarter tone.
    return -.25f;
  } else if (!hypothesisNoteNumber.has_value() &&
             measuredNoteNumber.has_value()) {
    // A false positive in the detection or unadvertently played note. Penalize
    // it like missing the pitch by a semitone.
    return -1.f;
  } else {
    // A PDF does not read as probabilities, but I think we're only interested
    // in the relative values, so let's just do this. The log of a normal
    // distribution is is proportional to the square of the error. Again, we're
    // just interested in how the probabilities compare, so we ignore all
    // scalars and just use
    const auto error = *hypothesisNoteNumber - *measuredNoteNumber;
    return -error * error;
  }
}

std::vector<float>
getObservationLikelihoods(const Melody &melody,
                          const std::optional<float> &measuredNoteNumber) {
  std::vector<float> llhs(melody.size() - 1u);
  for (auto i = 0u; i < melody.size() - 1u; ++i) {
    llhs[i] = getObservationLikelihood(melody[i].second, measuredNoteNumber);
  }
  return llhs;
}
} // namespace

MelodyRecognizer3::MelodyRecognizer3(Melody melody)
    : _melody(std::move(melody)), _numStates(_melody.size() - 1u),
      _transitionMatrix(getTransitionMatrix(_melody)),
      _priors(getInitialPriors(_numStates)), _newPriors(_priors.size()) {}

std::optional<size_t>
MelodyRecognizer3::tick(const std::optional<float> &measuredNoteNumber) {
  std::optional<float> maxProb;
  int maxProbState = noPitchState;
  const auto observationLlhs =
      getObservationLikelihoods(_melody, measuredNoteNumber);
  for (auto newState = 0u; newState < _numStates; ++newState) {
    auto maxProb = -std::numeric_limits<float>::max();
    auto maxProbIndex = 0u;
    for (auto oldState = 0u; oldState < _numStates; ++oldState) {
      const auto prob = _priors[oldState] +
                        _transitionMatrix[newState][oldState] +
                        observationLlhs[newState];
      if (prob > maxProb) {
        maxProb = prob;
        maxProbIndex = oldState;
      }
    }
    _newPriors[newState] = maxProb;
  }
  const auto winnerIndex =
      std::distance(_newPriors.begin(),
                    std::max_element(_newPriors.begin(), _newPriors.end()));
  _priors = _newPriors;
  return winnerIndex;
}

} // namespace saint