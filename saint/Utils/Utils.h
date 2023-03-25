#pragma once

#include "CommonTypes.h"

#include <map>
#include <string>

namespace saint {
namespace utils {
std::string getEnvironmentVariable(const char *);
bool getEnvironmentVariableAsBool(const char *);
bool isDebugBuild();
float getPitch(int noteNumber);
float getCrotchetsPerSample(float crotchetsPerSecond, int samplesPerSecond);
std::vector<SigPosWithCrotchet>
addBarInformation(const std::map<float, Fraction> &timeSignatures);
Position getCrotchetPosition(float crotchet,
                             const std::vector<SigPosWithCrotchet> &);
} // namespace utils
} // namespace saint
