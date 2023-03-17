#pragma once

#include "../Playhead.h"

#include <ringbuffer.hpp>

#include <array>

namespace saint {
class ProcessCallbackDrivenPlayhead : public Playhead {
public:
  ProcessCallbackDrivenPlayhead(int samplesPerSecond, float crotchetsPerSample);
  std::optional<float> incrementSampleCount(int numSamples) override;
  void mixMetronome(float *, int) override;
  std::optional<float> getTimeInCrotchets() override;

private:
  const float _crotchetsPerSample;
  const std::array<double, 2> _a;
  std::array<double, 2> _z = {0.f, 0.f};
  long long _sampleCount = 0;
  int _crotchetCount = -1;
};
} // namespace saint
