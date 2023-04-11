#include "IntervalGetter.h"
#include "DefaultIntervalGetter.h"
#include "IntervalGetterDebugCb.h"
#include "MelodyTracker/DefaultMelodyTracker.h"
#include "MelodyTracker/MelodyRecognizer/DefaultMelodyRecognizer.h"
#include "MelodyTracker/MelodyTracker.h"
#include "StochasticIntervalGetter.h"
#include "Utils.h"

#include <cassert>

namespace saint {
namespace {
std::vector<std::pair<float, std::optional<int>>>
toTimedNoteNumbers(const std::vector<IntervalSpan> &spans) {
  std::vector<std::pair<float, std::optional<int>>> timedNoteNumbers;
  for (const auto &span : spans) {
    timedNoteNumbers.emplace_back(
        span.beginCrotchet,
        span.playedNote.has_value()
            ? std::optional<int>{span.playedNote->noteNumber}
            : std::nullopt);
  }
  return timedNoteNumbers;
}

class DefaultClock : public Clock {
public:
  DefaultClock() : _creationTime(std::chrono::steady_clock::now()) {}

  std::chrono::milliseconds now() const override {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - _creationTime);
  }

private:
  const std::chrono::time_point<std::chrono::steady_clock> _creationTime;
};
} // namespace

constexpr auto useStochasticIntervalGetter = true;
std::shared_ptr<IntervalGetter>
IntervalGetter::createInstance(const std::vector<IntervalSpan> &spans,
                               const std::map<float, Fraction> &timeSignatures,
                               const std::optional<int> &samplesPerSecond,
                               const std::optional<float> &crotchetsPerSecond) {
  if (useStochasticIntervalGetter) {
    return std::make_shared<StochasticIntervalGetter>(
        spans, MelodyTracker::createInstance(toTimedNoteNumbers(spans)),
        std::make_unique<DefaultClock>(), timeSignatures);
  }
  if (utils::getEnvironmentVariableAsBool("SAINT_DEBUG_INTERVALGETTER") &&
      utils::isDebugBuild()) {
    assert(samplesPerSecond.has_value());
    assert(crotchetsPerSecond.has_value());
    const auto crotchetsPerSample = *crotchetsPerSecond / *samplesPerSecond;
    return std::make_shared<DefaultIntervalGetter>(
        spans, testUtils::getIntervalGetterDebugCb(crotchetsPerSample));
  } else {
    return std::make_shared<DefaultIntervalGetter>(spans, std::nullopt);
  }
}
} // namespace saint