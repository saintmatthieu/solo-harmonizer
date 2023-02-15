#include "BuiltinTicker.h"

namespace saint {
BuiltinTicker::BuiltinTicker(const AudioConfig &config)
    : _ticksPerBlock(
          static_cast<double>(config.ticksPerCrotchet) *
          config.samplesPerBlock *
          (config.crotchetsPerSecond == 0 ? 120 : config.crotchetsPerSecond) /
          config.samplesPerSecond) {}

void BuiltinTicker::incrementBlockCount() { ++_blockCount; }

std::optional<int> BuiltinTicker::getTick() const {
  return static_cast<int>(_ticksPerBlock * _blockCount + 0.5);
}
} // namespace saint