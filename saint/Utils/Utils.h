#pragma once

#include <optional>
#include <string>

namespace saint {
namespace utils {
std::optional<std::string> getEnvironmentVariable(const std::string &);
}
} // namespace saint
