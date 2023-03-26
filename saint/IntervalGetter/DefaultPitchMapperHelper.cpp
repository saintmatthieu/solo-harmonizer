#include "DefaultPitchMapperHelper.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_map>
#include <utility>

namespace saint {

namespace {
// aka mode of C
constexpr std::array<float, 9> aeolian{0.f, 2.f, 3.f,  5.f,
                                       7.f, 8.f, 10.f, 12.f};

float getAeolianSemi(float aeolianDegree) {
  const auto octave = std::floorf(aeolianDegree / 7.f);
  const auto wrappedDegree = aeolianDegree - octave * 7.f;
  const auto i = static_cast<int>(wrappedDegree);
  const auto fraction = wrappedDegree - i;
  const auto leftSemi = aeolian[i];
  const auto rightSemi = aeolian[i + 1];
  return leftSemi + fraction * (rightSemi - leftSemi) + octave * 12.f;
}

float getAeolianDegree(float aeolianSemi) {
  const auto octave = std::floorf(aeolianSemi / 12.f);
  const auto wrappedSemi = aeolianSemi - octave * 12.f;
  const auto rightIt = std::find_if(
      aeolian.begin(), aeolian.end(),
      [wrappedSemi](float aeolianSemi) { return aeolianSemi > wrappedSemi; });
  const auto leftSemi = *(rightIt - 1);
  const auto rightSemi = *rightIt;
  const auto fraction = (wrappedSemi - leftSemi) / (rightSemi - leftSemi);
  const auto integer =
      static_cast<float>(std::distance(aeolian.begin(), rightIt) - 1);
  return integer + fraction + octave * 7.f;
}

constexpr int getNumSharps(const Key &key) {
  return static_cast<float>(key.pc) - (key.mode == Mode::minor ? 3.f : 0.f);
}
} // namespace

float DefaultPitchMapperHelper::aeolianDegreeToSemi(float degree,
                                                    const Key &modulationKey) {
  const auto aeolianSemi = getAeolianSemi(degree);
  const auto numSharps = getNumSharps(modulationKey);
  const auto modulationShift = 7.f * numSharps;
  const auto modulationOctave = std::floorf(modulationShift / 12.f);
  const auto modulatedSemi =
      aeolianSemi + modulationShift - modulationOctave * 12;
  return modulatedSemi;
}

float DefaultPitchMapperHelper::semiToAeolianDegree(float modulatedSemi,
                                                    const Key &key) {

  const auto numSharps = getNumSharps(key);
  const auto modulationShift = 7.f * numSharps;
  const auto modulationOctave = std::floorf(modulationShift / 12.f);
  const auto aeolianSemi =
      modulatedSemi - modulationShift + modulationOctave * 12;
  const auto aeolianDegree = getAeolianDegree(aeolianSemi);
  return aeolianDegree;
}

float DefaultPitchMapperHelper::harmonize(float actualSemi, int intendedSemi,
                                          int harmonySemi, const Key &key) {
  const auto playedDegree =
      semiToAeolianDegree(static_cast<float>(intendedSemi), key);
  const auto harmoDegree =
      semiToAeolianDegree(static_cast<float>(harmonySemi), key);
  return harmonized(actualSemi, key, harmoDegree - playedDegree);
}

float DefaultPitchMapperHelper::harmonized(float semitone /*A = 0*/,
                                           const Key &key,
                                           float harmonyDegree) {
  const auto degree = semiToAeolianDegree(semitone, key);
  const auto harmonizedDegree = degree + harmonyDegree;
  const auto harmonizedSemi = aeolianDegreeToSemi(harmonizedDegree, key);
  return harmonizedSemi;
}

} // namespace saint