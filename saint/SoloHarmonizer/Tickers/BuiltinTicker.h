#pragma once

#include "../SoloHarmonizerTypes.h"
#include "ITicker.h"

namespace saint {
class BuiltinTicker : public ITicker {
public:
  BuiltinTicker(const AudioConfig &config)
      : _ticksPerBlock(
            static_cast<double>(config.ticksPerCrotchet) *
            config.samplesPerBlock *
            (config.crotchetsPerSecond == 0 ? 120 : config.crotchetsPerSecond) /
            config.samplesPerSecond) {}

  void incrementBlockCount() override;
  std::optional<int> getTick() const override;

private:
  const double _ticksPerBlock;
  int _blockCount = 0;
};
} // namespace saint
