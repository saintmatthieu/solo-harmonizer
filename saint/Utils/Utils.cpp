#include "Utils.h"
#include "CommonTypes.h"
#include <algorithm>
#include <cctype>
#include <cmath>

namespace saint {
namespace {
int getBeatsPerBar(const Fraction &timeSignature) {
  if (timeSignature.num > 3 && timeSignature.num % 3 == 0) {
    return timeSignature.num / 3;
  } else {
    return timeSignature.num;
  }
}
} // namespace

namespace utils {
std::string getEnvironmentVariable(const char *var) {
  char *buffer = nullptr;
  size_t size = 0;
  _dupenv_s(&buffer, &size, var);
  if (!buffer) {
    return "";
  } else {
    return std::string{buffer};
  }
}

bool getEnvironmentVariableAsBool(const char *var) {
  auto str = getEnvironmentVariable(var);
  std::transform(str.begin(), str.end(), str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return str == "1" || str == "true" || str == "on" || str == "yes" ||
         str == "y";
}

bool isDebugBuild() {
#ifdef NDEBUG
  return false;
#else
  return true;
#endif
}

float getPitch(int noteNumber) {
  return 440 * std::powf(2, (noteNumber - 69) / 12.f);
}

float getCrotchetsPerSample(float crotchetsPerSecond, int samplesPerSecond) {
  return (crotchetsPerSecond == 0 ? 120.f : crotchetsPerSecond) /
         static_cast<float>(samplesPerSecond);
}

Position
getCrotchetPosition(float crotchet,
                    const std::vector<SigPosWithCrotchet> &timeSignatures) {
  const auto sigBarIt =
      std::prev(std::find_if(timeSignatures.begin(), timeSignatures.end(),
                             [crotchet](const SigPosWithCrotchet &pos) {
                               return pos.crotchet > crotchet;
                             }));
  const auto crotchetsFromSigBar = crotchet - sigBarIt->crotchet;
  const auto sigBar = sigBarIt->barIndex;
  const auto &sig = sigBarIt->timeSignature;
  const auto crotchetsPerBar = 4.f * sig.num / sig.den;
  const auto barsFromSigBar =
      static_cast<int>(crotchetsFromSigBar / crotchetsPerBar);
  const auto crotchetsFromBar =
      crotchetsFromSigBar - barsFromSigBar * crotchetsPerBar;
  const auto beatsPerBar = getBeatsPerBar(sig);
  const auto beatsPerCrotchet = beatsPerBar / crotchetsPerBar;
  return {sigBar + barsFromSigBar, beatsPerCrotchet * crotchetsFromBar};
}

std::vector<SigPosWithCrotchet>
addBarInformation(const std::map<float, Fraction> &timeSignatures) {
  std::vector<SigPosWithCrotchet> positions;
  auto barIndex = 0;
  auto barCrotchet = 0.f;
  auto crotchetsPerBar = 4.f;
  for (auto it = timeSignatures.begin(); it != timeSignatures.end(); ++it) {
    SigPosWithCrotchet position;
    position.crotchet = it->first;
    position.timeSignature = it->second;
    barIndex +=
        static_cast<int>((position.crotchet - barCrotchet) / crotchetsPerBar);
    position.barIndex = barIndex;
    crotchetsPerBar =
        4.f * position.timeSignature.num / position.timeSignature.den;
    barCrotchet = position.crotchet;
    positions.push_back(position);
  }
  return positions;
}

} // namespace utils
} // namespace saint
