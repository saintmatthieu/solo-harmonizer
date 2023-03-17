#include "ProcessCallbackDrivenPlayhead.h"

#include <cassert>
#include <cmath>
#include <optional>

namespace saint {

namespace {
constexpr auto resonanceHz = 1000;
constexpr auto gainCoef = 0.01;
constexpr auto twoPi = 6.283185307179586;
constexpr auto decayTime = 1.f;
constexpr auto decayDb = -60.f;

std::array<double, 2> getCoefs(int samplesPerSecond) {
  const auto resonanceRadians = twoPi * resonanceHz / samplesPerSecond;
  const auto r = std::powf(10.f, decayDb / (decayTime * samplesPerSecond));
  const auto a1 = -2 * r * std::cos(resonanceRadians);
  const auto a2 = r * r;
  return {a1, a2};
}
} // namespace

ProcessCallbackDrivenPlayhead::ProcessCallbackDrivenPlayhead(
    int samplesPerSecond, float crotchetsPerSample)
    : _crotchetsPerSample(crotchetsPerSample), _a(getCoefs(samplesPerSecond)) {}

std::optional<float>
ProcessCallbackDrivenPlayhead::incrementSampleCount(int numSamples) {
  _sampleCount += numSamples;
  return getTimeInCrotchets();
}

void ProcessCallbackDrivenPlayhead::mixMetronome(float *audio, int numSamples) {
  for (auto i = 0u; i < static_cast<size_t>(numSamples); ++i) {
    const auto newCrotchetCount = static_cast<int>(
        _crotchetsPerSample *
        (_sampleCount + static_cast<decltype(_sampleCount)>(i)));
    const auto isTick = newCrotchetCount > _crotchetCount;
    _crotchetCount = newCrotchetCount;
    const auto x = isTick ? 1.0 : 0.0;
    const auto y = gainCoef * x - _a[0] * _z[0] - _a[1] * _z[1];
    _z[1] = _z[0];
    _z[0] = y;
    audio[i] += static_cast<float>(y);
  }
}

std::optional<float> ProcessCallbackDrivenPlayhead::getTimeInCrotchets() {
  return static_cast<float>(_sampleCount) * _crotchetsPerSample;
}
} // namespace saint