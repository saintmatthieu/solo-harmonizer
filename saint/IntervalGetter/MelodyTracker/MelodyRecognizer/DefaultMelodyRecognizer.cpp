#include "DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyTrackerHelper.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <optional>
#include <set>

namespace saint {
std::unique_ptr<MelodyRecognizer> MelodyRecognizer::createInstance(
    const std::vector<std::pair<float, std::optional<int>>> &melody) {
  return std::make_unique<DefaultMelodyRecognizer>(
      ObservationLikelihoodGetter::createInstance(melody), melody);
}

DefaultMelodyRecognizer::DefaultMelodyRecognizer(
    std::unique_ptr<ObservationLikelihoodGetter> likelihoodGetter,
    const std::vector<std::pair<float, std::optional<int>>> &melody)
    : _likelihoodGetter(std::move(likelihoodGetter)),
      _melody(MelodyTrackerHelper::getMelody(melody)),
      _intervals(MelodyTrackerHelper::getIntervals(_melody)),
      _uniqueIntervals(MelodyTrackerHelper::getUniqueIntervals(_intervals)) {}

bool DefaultMelodyRecognizer::onNoteOff(
    const std::vector<std::pair<std::chrono::milliseconds, float>>
        &noteonSamples) {
  const auto obsLikelihoods =
      _likelihoodGetter->getObservationLogLikelihoods(noteonSamples);
  return false;
}

size_t DefaultMelodyRecognizer::getNextNoteIndex() {
  assert(_nextNoteIndex.has_value());
  return _nextNoteIndex.has_value() ? *_nextNoteIndex : 0u;
}
} // namespace saint