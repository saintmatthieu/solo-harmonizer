#pragma once

#include "../SoloHarmonizerTypes.h"
#include "IPlayhead.h"

namespace saint {
class BuiltinPlayhead : public IPlayhead {
public:
  BuiltinPlayhead(const AudioConfig &config);
  void incrementSampleCount(int numSamples);
  std::optional<double> getTimeInCrotchets() const override;

private:
  const double _crotchetsPerSample;
  int _sampleCount = 0;
};
} // namespace saint
