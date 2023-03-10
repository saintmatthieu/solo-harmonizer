#pragma once

#include "../Playhead.h"
#include "../SoloHarmonizerTypes.h"

#include <atomic>

namespace saint {
class ProcessCallbackDrivenPlayhead : public Playhead {
public:
  ProcessCallbackDrivenPlayhead(float crotchetsPerSample);
  void incrementSampleCount(int numSamples) override;
  void mixMetronome(float *, int) override;
  std::optional<float> getTimeInCrotchets() const override;

private:
  const float _crotchetsPerSample;
  long long _sampleCount = 0;
};
} // namespace saint
