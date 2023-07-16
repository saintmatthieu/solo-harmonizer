#include "IntervalGetter.h"
#include "DefaultIntervalGetter.h"
#include "IntervalGetterDebugCb.h"
#include "MelodyRecognizer3/MelodyRecognizer3.h"
#include "StochasticIntervalGetter.h"
#include "Utils.h"

#include <cassert>

namespace saint {
namespace {
std::vector<std::pair<float, std::optional<int>>>
toTimedNoteNumbers(const std::vector<IntervalSpan> &spans) {
  std::vector<std::pair<float, std::optional<int>>> timedNoteNumbers;
  const auto firstNoteIt =
      std::find_if(spans.begin(), spans.end(), [](const IntervalSpan &span) {
        return span.playedNote.has_value();
      });
  for (auto it = firstNoteIt; it != spans.end(); ++it) {
    timedNoteNumbers.emplace_back(
        it->beginCrotchet, it->playedNote.has_value()
                               ? std::optional<int>{it->playedNote->noteNumber}
                               : std::nullopt);
  }
  return timedNoteNumbers;
}
} // namespace

constexpr auto useStochasticIntervalGetter = true;
std::shared_ptr<IntervalGetter>
IntervalGetter::createInstance(const std::vector<IntervalSpan> &spans,
                               const std::map<float, Fraction> &timeSignatures,
                               const std::optional<int> &samplesPerSecond,
                               const std::optional<float> &crotchetsPerSecond) {
  if (useStochasticIntervalGetter) {
    return std::make_shared<StochasticIntervalGetter>(
        spans, std::make_unique<MelodyRecognizer3>(toTimedNoteNumbers(spans)),
        timeSignatures);
  }
  if (utils::getEnvironmentVariableAsBool("SAINT_DEBUG_INTERVALGETTER") &&
      utils::isDebugBuild()) {
    assert(samplesPerSecond.has_value());
    assert(crotchetsPerSecond.has_value());
    const auto crotchetsPerSample = *crotchetsPerSecond / *samplesPerSecond;
    return std::make_shared<DefaultIntervalGetter>(
        spans, testUtils::getIntervalGetterDebugCb(crotchetsPerSample,
                                                   *samplesPerSecond));
  } else {
    return std::make_shared<DefaultIntervalGetter>(spans, std::nullopt);
  }
}
} // namespace saint