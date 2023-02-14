#include "BuiltinTicker.h"

namespace saint {
void BuiltinTicker::incrementBlockCount() { ++_blockCount; }

std::optional<int> BuiltinTicker::getTick() const {
  return static_cast<int>(_ticksPerBlock * _blockCount + 0.5);
}
} // namespace saint