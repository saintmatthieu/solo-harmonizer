#include "KeyRecognizerHelper.h"

namespace saint {

namespace {
float getCrotchetsPerBar(const Fraction &sig) {
  return static_cast<float>(sig.num) / static_cast<float>(sig.den) * 4.f;
}
} // namespace

std::map<size_t, size_t>
KeyRecognizerHelper::groupIndicesByBar(const std::vector<float> &crotchets,
                                       const std::vector<SigPos> &sigs) {
  std::map<size_t, size_t> groups;
  auto sigIt = sigs.begin();
  auto nextSigIt = sigIt + 1;
  auto barIndex = 0;
  auto sigCrotchet = 0.f;
  auto i = 0u;
  auto crotchetsPerBar = getCrotchetsPerBar(sigIt->timeSignature);
  std::optional<float> nextSigCrotchet;
  if (nextSigIt != sigs.end()) {
    nextSigCrotchet = nextSigIt->barIndex * crotchetsPerBar;
  }
  while (i < crotchets.size()) {
    if (nextSigCrotchet.has_value() && crotchets[i] >= *nextSigCrotchet) {
      sigCrotchet = *nextSigCrotchet;
      ++nextSigIt;
      ++sigIt;
      crotchetsPerBar = getCrotchetsPerBar(sigIt->timeSignature);
      if (nextSigIt != sigs.end()) {
        *nextSigCrotchet =
            sigCrotchet +
            (nextSigIt->barIndex - sigIt->barIndex) * crotchetsPerBar;
      } else {
        nextSigCrotchet.reset();
      }
    }
    const auto sigBarIndex = sigIt->barIndex;
    while (i < crotchets.size()) {
      const auto crotchetsSinceSig = crotchets[i] - sigCrotchet;
      const auto barsSinceSig =
          static_cast<int>(crotchetsSinceSig / crotchetsPerBar);
      const auto newBarIndex = sigBarIndex + barsSinceSig;
      if (newBarIndex > barIndex) {
        break;
      }
      ++i;
    }
    if (groups.empty() || std::prev(groups.end())->second != i) {
      groups[barIndex] = i;
    }
    ++barIndex;
  }
  return groups;
}
} // namespace saint