#include "MelodyRecognizer3.h"

#include <cmath>
#include <limits>

namespace saint {
using Melody = MelodyRecognizer3::Melody;

namespace {
constexpr auto noPitchState = std::numeric_limits<int>::min();

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
  constexpr auto transitionsToNextConditionalLlh = .9f;
  for (auto i = 0u; i < N; ++i) {
    // Likelihood that we stay at the same index.
    const auto noteDuration = melody[i + 1u].first - melody[i].first;
    const auto relativeStayLl = noteDuration / totalDuration;
    for (auto j = 0u; j < N; ++j) {
      if (i == j) { // Stay in the same state
        matrix[i][j] = std::logf(relativeStayLl);
      } else if (j == i + 1u) {
        matrix[i][j] =
            std::logf((1.f - relativeStayLl) * transitionsToNextConditionalLlh);
      } else {
        matrix[i][j] = std::logf((1.f - relativeStayLl) *
                                 (1.f - transitionsToNextConditionalLlh));
      }
    }
  }
  return matrix;
}

std::vector<float> getInitialPriors(size_t numStates) {
  // For now initially just give equal chance to all states. Maybe priviledging
  // index 0, assuming that users typically begin at the start, could be an
  // option. But that certainly shouldn't be a constraint.
  const auto prior = -std::logf(static_cast<size_t>(numStates));
  std::vector<float> priors(numStates);
  for (auto i = 0u; i < numStates; ++i) {
    priors[i] = prior;
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
  for (auto i = 0u; i < _numStates; ++i) {
    auto maxProb = -std::numeric_limits<float>::max();
    auto maxProbIndex = 0u;
    for (auto j = 0u; j < _numStates; ++j) {
      const auto prob =
          _priors[j] + _transitionMatrix[i][j] + observationLlhs[j];
      if (prob > maxProb) {
        maxProb = prob;
        maxProbIndex = j;
      }
    }
    _newPriors[i] = maxProb;
  }
  const auto winnerIndex =
      std::distance(_newPriors.begin(),
                    std::max_element(_newPriors.begin(), _newPriors.end()));
  std::copy(_newPriors.begin(), _newPriors.end(), _priors.begin());
  return winnerIndex;
}

} // namespace saint