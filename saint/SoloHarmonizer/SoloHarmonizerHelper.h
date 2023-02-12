#pragma once

#include <filesystem>
#include <spdlog/common.h>

namespace saint {
std::filesystem::path getLogDir();
std::filesystem::path generateLogFilename(const std::string &loggerName);
spdlog::level::level_enum getLogLevelFromEnv();
} // namespace saint
