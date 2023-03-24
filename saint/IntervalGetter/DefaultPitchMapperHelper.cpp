#include "DefaultPitchMapperHelper.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>
#include <utility>

namespace saint {

namespace {

// aka mode of A
constexpr std::array<float, 9> aeolian{0.f, 2.f, 3.f,  5.f,
                                       7.f, 8.f, 10.f, 12.f};

float aeolianDegreeToSemi(float degree, const Key &key) {
  const auto degreeIndex = degree - 1.f;
  const auto octave = std::floorf(degreeIndex / 7.f);
  const auto wrappedDegreeIndex = degreeIndex - octave * 7.f;
  const auto i = static_cast<int>(wrappedDegreeIndex);
  const auto fraction = wrappedDegreeIndex - i;
  const auto leftSemi = aeolian[i];
  const auto rightSemi = aeolian[i + 1];
  const auto semi = leftSemi + fraction * (rightSemi - leftSemi);
  const auto numSharps =
      static_cast<float>(key.pc) - (key.mode == Mode::minor ? 3.f : 0.f);
  const auto unmodulatedSemi =
      std::fmodf(semi + 7.f * numSharps, 12.f) + octave * 12.f;
  return unmodulatedSemi;
}

float semiToAeolianDegree(float semi, // from some A
                          const Key &key) {
  const auto numSharps =
      static_cast<float>(key.pc) - (key.mode == Mode::minor ? 3.f : 0.f);
  const auto modulatedSemi = semi - 7.f * numSharps;
  const auto modulatedOctave = std::floorf(modulatedSemi / 12.f);
  const auto wrappedSemi = modulatedSemi - modulatedOctave * 12.f;
  const auto rightIt = std::find_if(
      aeolian.begin(), aeolian.end(),
      [wrappedSemi](float ionianSemi) { return ionianSemi > wrappedSemi; });
  const auto leftSemi = *(rightIt - 1);
  const auto rightSemi = *rightIt;
  const auto fraction = (wrappedSemi - leftSemi) / (rightSemi - leftSemi);
  const auto wrappedDegree =
      static_cast<float>(std::distance(aeolian.begin(), rightIt) - 1) +
      fraction;
  const auto octave = std::floorf(semi / 12.f);
  return wrappedDegree + octave * 7.f + 1.f;
}
} // namespace

float DefaultPitchMapperHelper::harmonize(float actualSemi, int intendedSemi,
                                          int harmonySemi, const Key &key) {
  const auto playedDegree =
      semiToAeolianDegree(static_cast<float>(intendedSemi), key);
  const auto harmoDegree =
      semiToAeolianDegree(static_cast<float>(harmonySemi), key);
  const auto degreeDiff =
      static_cast<int>(harmoDegree) - static_cast<int>(playedDegree);
  return harmonized(actualSemi, key, degreeDiff);
}

float DefaultPitchMapperHelper::harmonized(float semitone /*A = 0*/,
                                           const Key &key, int harmonyDegree) {
  const auto degree = semiToAeolianDegree(semitone, key);
  const auto harmonizedDegree = degree + static_cast<float>(harmonyDegree);
  const auto harmonizedCents = aeolianDegreeToSemi(harmonizedDegree, key);
  return harmonizedCents;
}

} // namespace saint