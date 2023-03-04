#include "Utils.h"

namespace saint {
namespace utils {
std::optional<std::string> getEnvironmentVariable(const std::string &var) {
  char *buffer = nullptr;
  size_t size = 0;
  _dupenv_s(&buffer, &size, "SAINT_LOG_LEVEL");
  if (!buffer) {
    return std::nullopt;
  } else {
    return std::string{buffer};
  }
}
} // namespace utils
} // namespace saint