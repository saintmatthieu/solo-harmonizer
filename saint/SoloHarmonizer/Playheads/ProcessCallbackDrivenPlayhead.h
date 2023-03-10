#pragma once

#include "../Playhead.h"
#include "../SoloHarmonizerTypes.h"

#include <atomic>

namespace saint {
class ProcessCallbackDrivenPlayhead : public Playhead {
public:
  ProcessCallbackDrivenPlayhead(float crotchetsPerSample);
  void incrementSampleCount(int numSamples) override;
  bool execute(PlayheadCommand) override;
  std::optional<float> getTimeInCrotchets() const override;

private:
  const float _crotchetsPerSample;
  std::atomic<long long> _sampleCount = 0;
  bool _paused = true;
};
} // namespace saint
