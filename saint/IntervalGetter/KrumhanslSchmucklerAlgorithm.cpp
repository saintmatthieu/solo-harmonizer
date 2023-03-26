#include "KrumhanslSchmucklerAlgorithm.h"

#include <algorithm>
#include <array>
#include <unordered_set>

namespace saint {
namespace {
constexpr std::array<float, 12> cMajorProfile{5.f, 2.f,  3.5f, 2.f,  4.5f, 4.f,
                                              2.f, 4.5f, 2.f,  3.5f, 1.5f, 4.f};
constexpr std::array<float, 12> cMinorProfile{5.f, 2.f,  3.5f, 4.5f, 2.f,  4.f,
                                              2,   4.5f, 3.5f, 2.f,  1.5f, 4.f};
} // namespace

std::vector<std::pair<Key, float>>
KrumhanslSchmucklerAlgorithm::getKeyLikelihoods(
    const std::unordered_set<int> &noteNumbers) {
  std::unordered_set<int> ionianPcs;
  for (auto nn : noteNumbers) {
    const auto relative = nn - 60;
    const auto octave =
        static_cast<int>(std::floorf(static_cast<float>(relative) / 12.f));
    ionianPcs.insert(relative - octave * 12);
  }
  std::vector<std::pair<Key, float>> crossProducts(24);
  for (auto numSharps = 0; numSharps < 12; ++numSharps) {
    auto majorResult = 0.f;
    auto minorResult = 0.f;
    const auto rootIndex = 12 * 7 - numSharps * 7;
    for (auto pc : ionianPcs) {
      const auto scaleIndex = (pc + rootIndex) % 12;
      majorResult += cMajorProfile[scaleIndex];
      minorResult += cMinorProfile[scaleIndex];
    }
    const auto pc = static_cast<PC>(numSharps);
    crossProducts[numSharps * 2] = {{pc, Mode::major}, majorResult};
    crossProducts[numSharps * 2 + 1] = {{pc, Mode::minor}, minorResult};
  }
  std::sort(crossProducts.begin(), crossProducts.end(),
            [](const auto &a, const auto &b) { return a.second > b.second; });
  return crossProducts;
}

Key KrumhanslSchmucklerAlgorithm::getMostLikelyKey(
    const std::unordered_set<int> &noteNumbers) {
  return getKeyLikelihoods(noteNumbers)[0].first;
}

} // namespace saint
