#include "KeyRecognizer.h"
#include "CommonTypes.h"
#include "KeyRecognizerHelper.h"
#include "KrumhanslSchmucklerAlgorithm.h"
#include "Utils.h"

#include <algorithm>
#include <iterator>
#include <xlocale>

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
    std::unordered_set<int> noteNumbers;
    while (spanIndex < spanEndIndex) {
      const auto &span = spans[spanIndex++];
      for (auto note : span.overlappingNotes) {
        noteNumbers.insert(note);
      }
      if (span.playedNote.has_value()) {
        const auto &note = *span.playedNote;
        noteNumbers.insert(note.noteNumber);
        if (note.interval.has_value()) {
          noteNumbers.insert(note.noteNumber + *note.interval);
        }
      }
    }
    if (noteNumbers.empty()) {
      keys.emplace_back(spanIndex, prevKey);
    } else {
      const auto likelihoods =
          KrumhanslSchmucklerAlgorithm::getKeyLikelihoods(noteNumbers);
      keys.emplace_back(spanIndex, likelihoods[0].first);
    }
  }
  return keys;
}
} // namespace

KeyRecognizer::KeyRecognizer(const std::vector<IntervalSpan> &spans,
                             const std::map<float, Fraction> &sigFractions)
    : _spans(spans), _spanKeys(getSpanKeys(spans, sigFractions)) {}

Key KeyRecognizer::getKey(float crotchet) {
  while (_spanIndex < _spans.size() &&
         _spans[_spanIndex].beginCrotchet <= crotchet) {
    ++_spanIndex;
  }
  --_spanIndex;
  const auto keyIt = std::find_if(_spanKeys.begin(), _spanKeys.end(),
                                  [this](const std::pair<size_t, Key> &entry) {
                                    const auto lastSpanIndex = entry.first;
                                    return _spanIndex < lastSpanIndex;
                                  });
  return keyIt->second;
}
} // namespace saint