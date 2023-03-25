#include "KeyRecognizer.h"
#include "CommonTypes.h"
#include "KeyRecognizerHelper.h"
#include "KrumhanslSchmucklerAlgorithm.h"
#include "Utils.h"

#include <algorithm>
#include <iterator>

namespace saint {

namespace {
std::vector<std::pair<size_t, Key>>
getSpanKeys(const std::vector<IntervalSpan> &spans,
            const std::map<float, Fraction> &sigFractions) {
  const auto sigsWithBar = utils::addBarInformation(sigFractions);
  std::vector<SigPos> sigs(sigsWithBar.size());
  std::transform(sigsWithBar.begin(), sigsWithBar.end(), sigs.begin(),
                 [](const SigPosWithCrotchet &wc) -> SigPos {
                   return {wc.barIndex, wc.timeSignature};
                 });
  std::vector<float> crotchets(spans.size());
  std::transform(spans.begin(), spans.end(), crotchets.begin(),
                 [](const IntervalSpan &span) { return span.beginCrotchet; });
  const auto groups = KeyRecognizerHelper::groupIndicesByBar(crotchets, sigs);
  auto spanIndex = 0;
  std::vector<std::pair<size_t, Key>> keys;
  Key prevKey = {PC::C, Mode::major};
  for (const auto group : groups) {
    const auto barIndex = static_cast<int>(group.first);
    const auto spanEndIndex = static_cast<int>(group.second);
    std::vector<int> noteNumbers;
    while (spanIndex < spanEndIndex) {
      const auto &span = spans[spanIndex++];
      std::copy(span.overlappingNotes.begin(), span.overlappingNotes.end(),
                std::back_inserter(noteNumbers));
    }
    if (noteNumbers.empty()) {
      keys.emplace_back(spanIndex, prevKey);
    } else {
      const auto likelihoods =
          KrumhanslSchmucklerAlgorithm::getLikelihoods(noteNumbers);
      keys.emplace_back(spanIndex, likelihoods[0].first);
    }
  }
  return keys;
}
} // namespace

KeyRecognizer::KeyRecognizer(const std::vector<IntervalSpan> &spans,
                             const std::map<float, Fraction> &sigFractions)
    : _spans(spans), _spanKeys(getSpanKeys(spans, sigFractions)) {}

Key KeyRecognizer::getKey(
    const std::vector<IntervalSpan>::const_iterator &spanIt) {
  const auto i = std::distance(_spans.begin(), spanIt);
  const auto keyIt =
      std::prev(std::find_if(_spanKeys.begin(), _spanKeys.end(),
                             [i](const std::pair<size_t, Key> &entry) {
                               const auto lastSpanIndex = entry.first;
                               return i > lastSpanIndex;
                             }));
  return keyIt->second;
}
} // namespace saint