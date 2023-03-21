#include "IntervalGetter.h"
#include "DefaultIntervalGetter.h"
#include "IntervalGetterDebugCb.h"
#include "Utils.h"

#include <cassert>

namespace saint {
std::shared_ptr<IntervalGetter>
IntervalGetter::createInstance(const std::vector<IntervalSpan> &spans,
                               const std::optional<int> &samplesPerSecond,
                               const std::optional<float> &crotchetsPerSecond) {
  // TODO: No need to wrap IntervalGetter
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