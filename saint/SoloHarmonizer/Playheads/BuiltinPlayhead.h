#pragma once

#include "../SoloHarmonizerTypes.h"
#include "IPlayhead.h"

namespace saint {
class BuiltinPlayhead : public IPlayhead {
public:
  BuiltinPlayhead(const AudioConfig &config);
  void incrementSampleCount(int numSamples) override;
  std::optional<double> getTimeInCrotchets() const override;

private:
  const double _ticksPerSample;
  int _sampleCount = 0;
};
} // namespace saint
