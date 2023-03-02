#pragma once

#include <memory>

namespace DavidCNAntonia {
class IPitchShifter {
public:
  static std::unique_ptr<IPitchShifter>
  createInstance(int numChannels, double sampleRate, int samplesPerBlock);

  virtual ~IPitchShifter() = default;

  virtual void setFormantPreserving(bool shouldPreserveFormants) = 0;

  virtual int getLatency() = 0;

  /** Pitch shift a juce::AudioBuffer<float>
   */
  virtual void processBuffer(float *const *audio, int numberOfChannels,
                             int numberOfSamples) = 0;

  /** Set the wet/dry mix as a % value.
   */
  virtual void setMixPercentage(float newPercentage) = 0;

  /** Set the pitch shift in semitones.
   */
  virtual void setSemitoneShift(float newShift) = 0;

  /** Get the % value of the wet/dry mix.
   */
  virtual float getMixPercentage() = 0;

  /** Get the pitch shift in semitones.
   */
  virtual float getSemitoneShift() = 0;

  /** Get the estimated latency. This is an average guess of latency with no
   * pitch shifting but can vary by a few buffers. Changing the pitch shift can
   * cause less or more latency.
   */
  virtual int getLatencyEstimationInSamples() = 0;
};
} // namespace DavidCNAntonia
