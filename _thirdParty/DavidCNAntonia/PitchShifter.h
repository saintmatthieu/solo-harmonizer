#pragma once

#include "RingBuffer.h"
#include <juce_dsp/juce_dsp.h>
#include <rubberband/RubberBandStretcher.h>

namespace DavidCNAntonia {
class PitchShifter {
public:
  static const RubberBand::RubberBandStretcher::Options defaultOptions =
      RubberBand::RubberBandStretcher::Option::OptionProcessRealTime +
      RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency +
      RubberBand::RubberBandStretcher::Option::OptionTransientsSmooth +
      RubberBand::RubberBandStretcher::Option::OptionPhaseIndependent +
      RubberBand::RubberBandStretcher::Option::OptionFormantPreserved +
      RubberBand::RubberBandStretcher::Option::OptionChannelsTogether +
      RubberBand::RubberBandStretcher::Option::OptionWindowShort;

  /** Setup the pitch shifter. By default the shifter will be setup so that the
   * dry signal isn't delayed to be given a somewhat similar latency to the wet
   * signal - this is not accurate when enabled! By enabling minLatency some
   * latency can be reduced with the expense of potential tearing during
   * modulation with a change of the pitch parameter.
   */
  PitchShifter(int numChannels, double sampleRate, int samplesPerBlock,
               std::optional<RubberBand::RubberBandStretcher::Options> opts);

  void setFormantPreserving(bool shouldPreserveFormants);

  int getLatency();

  /** Pitch shift a juce::AudioBuffer<float>
   */
  void processBuffer(juce::dsp::AudioBlock<float> &block);

  /** Set the wet/dry mix as a % value.
   */
  void setMixPercentage(float newPercentage);

  /** Set the pitch shift in semitones.
   */
  void setSemitoneShift(float newShift);

  /** Get the % value of the wet/dry mix.
   */
  float getMixPercentage();

  /** Get the pitch shift in semitones.
   */
  float getSemitoneShift();

  /** Get the estimated latency. This is an average guess of latency with no
   * pitch shifting but can vary by a few buffers. Changing the pitch shift can
   * cause less or more latency.
   */
  int getLatencyEstimationInSamples();

private:
  std::unique_ptr<RubberBand::RubberBandStretcher> rubberband;
  RingBuffer input, output;
  int maxSamples, initLatency, bufferFail, smallestAcceptableSize,
      largestAcceptableSize;
  float oldPitch, pitchParam, mixParam, newPitch;
  std::unique_ptr<juce::dsp::DryWetMixer<float>> dryWet;
  juce::SmoothedValue<float> timeSmoothing, mixSmoothing, pitchSmoothing;
  bool formantPreserving;
  int latencyInSamples = 0, samplesToSkip = 0, readSpace;
  size_t reqSamples;
};
} // namespace DavidCNAntonia