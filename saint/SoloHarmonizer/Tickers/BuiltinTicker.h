#pragma once

#include "../SoloHarmonizerTypes.h"
#include "ITicker.h"

namespace saint {
class BuiltinTicker : public ITicker {
public:
  BuiltinTicker(const AudioConfig &config);
  void incrementSampleCount(int numSamples) override;
  std::optional<int> getTick() const override;

private:
  const double _ticksPerSample;
  int _sampleCount = 0;
};
} // namespace saint
