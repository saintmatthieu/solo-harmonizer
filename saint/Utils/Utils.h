#pragma once

#include <string>

namespace saint {
namespace utils {
std::string getEnvironmentVariable(const char *);
bool getEnvironmentVariableAsBool(const char *);
bool isDebugBuild();
float getPitch(int noteNumber);
} // namespace utils
} // namespace saint
