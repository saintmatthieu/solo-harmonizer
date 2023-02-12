#include "SoloHarmonizerHelper.h"

#include <array>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iterator>
#include <stdlib.h>

namespace saint {
std::filesystem::path getLogDir() {
#ifdef _WIN32
  const auto macro = "LOCALAPPDATA";
#else
#error please find out
#endif
  char *buffer = nullptr;
  size_t size = 0;
  _dupenv_s(&buffer, &size, macro);
  return std::filesystem::path{buffer}.append("saint");
}

std::filesystem::path generateLogFilename(const std::string &loggerName) {
  time_t now;
  time(&now);
  char buf[sizeof("2011-10-08T07:07:09Z")];
  strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&now));
  std::string rawFilename{buf};
  std::string compatibleFilename;
  std::transform(rawFilename.begin(), rawFilename.end(),
                 std::back_inserter(compatibleFilename),
                 [](auto c) { return c == ':' ? '-' : c; });
  auto logDir = getLogDir();
  if (!std::filesystem::exists(logDir)) {
    std::filesystem::create_directory(logDir);
  }
  return logDir.append(compatibleFilename + "_" + loggerName + ".log");
}

spdlog::level::level_enum getLogLevelFromEnv() {
  constexpr auto defaultLogLevel = spdlog::level::info;
  char *buffer = nullptr;
  size_t size = 0;
  _dupenv_s(&buffer, &size, "SAINT_LOG_LEVEL");
  if (!buffer) {
    return defaultLogLevel;
  }
  const std::string str{buffer};
  if (str == "trace") {
    return spdlog::level::trace;
  } else if (str == "debug") {
    return spdlog::level::debug;
  } else if (str == "info") {
    return spdlog::level::info;
  } else if (str == "warn") {
    return spdlog::level::warn;
  } else if (str == "err") {
    return spdlog::level::err;
  } else if (str == "critical") {
    return spdlog::level::critical;
  } else if (str == "off") {
    return spdlog::level::off;
  } else {
    // unrecognized
    return defaultLogLevel;
  }
}
} // namespace saint
