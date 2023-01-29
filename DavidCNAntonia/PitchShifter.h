#pragma once

#include "RingBuffer.h"
#include <juce_dsp/juce_dsp.h>
#include <rubberband/RubberBandStretcher.h>

class PitchShifter {
public:
  /** Setup the pitch shifter. By default the shifter will be setup so that the
   * dry signal isn't delayed to be given a somewhat similar latency to the wet
   * signal - this is not accurate when enabled! By enabling minLatency some
   * latency can be reduced with the expense of potential tearing during
   * modulation with a change of the pitch parameter.
   */
  PitchShifter(int numChannels, double sampleRate, int samplesPerBlock) {
    rubberband = std::make_unique<RubberBand::RubberBandStretcher>(
        sampleRate, numChannels,
        RubberBand::RubberBandStretcher::Option::OptionProcessRealTime +
            RubberBand::RubberBandStretcher::Option::
                OptionPitchHighConsistency +
            RubberBand::RubberBandStretcher::Option::OptionTransientsSmooth +
            RubberBand::RubberBandStretcher::Option::OptionPhaseIndependent +
            RubberBand::RubberBandStretcher::Option::OptionFormantPreserved +
            RubberBand::RubberBandStretcher::Option::OptionChannelsTogether +
            RubberBand::RubberBandStretcher::Option::OptionWindowShort,
        1.0, 1.0);

    initLatency = (int)rubberband->getLatency();
    maxSamples = sampleRate / 1000.0 * 4.0;

    input.initialise(numChannels, sampleRate);
    output.initialise(numChannels, sampleRate);

    for (int sample = 0; sample < rubberband->getPreferredStartPad();
         ++sample) { // Loop to push samples to input buffer.
      for (int channel = 0; channel < numChannels; channel++) {
        input.pushSample(0.0, channel);
      }
    }

    samplesToSkip = (int)rubberband->getStartDelay();

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = numChannels;
    spec.sampleRate = sampleRate;

    timeSmoothing.reset(sampleRate, 0.05);
    mixSmoothing.reset(sampleRate, 0.1);
    pitchSmoothing.reset(sampleRate, 0.0);

    pitchSmoothing.setCurrentAndTargetValue(1.0);
    timeSmoothing.setCurrentAndTargetValue(1.0);

    smallestAcceptableSize = maxSamples * 0.5;
    largestAcceptableSize = maxSamples * 1.5;

    latencyInSamples = (initLatency + maxSamples);

    dryWet =
        std::make_unique<juce::dsp::DryWetMixer<float>>(latencyInSamples * 2);
    dryWet->prepare(spec);
    dryWet->setWetLatency(latencyInSamples);

    formantPreserving = true;
  }

  ~PitchShifter() {}

  void setFormantPreserving(bool shouldPreserveFormants) {
    if (shouldPreserveFormants != formantPreserving) {
      formantPreserving = shouldPreserveFormants;
      rubberband->setFormantOption(
          (shouldPreserveFormants)
              ? RubberBand::RubberBandStretcher::Option::OptionFormantPreserved
              : RubberBand::RubberBandStretcher::Option::OptionFormantShifted);
    }
  }

  int getLatency() { return latencyInSamples; }

  /** Pitch shift a juce::AudioBuffer<float>
   */
  void processBuffer(juce::dsp::AudioBlock<float> &block) {

    pitchSmoothing.setTargetValue(
        powf(2.0, pitchParam /
                      12)); // Convert semitone value into pitch scale value.

    if (!pitchSmoothing.isSmoothing()) {
      if (pitchSmoothing.getCurrentValue() <= 1.0) {
        smallestAcceptableSize = maxSamples * 0.5;
        largestAcceptableSize = maxSamples * 1.5;
      } else if (pitchSmoothing.getCurrentValue() > 1.0) {
        smallestAcceptableSize = maxSamples * 0.5;
        largestAcceptableSize = maxSamples * 2.5;
      }
    }

    dryWet->pushDrySamples(block);

    for (int sample = 0; sample < block.getNumSamples();
         ++sample) { // Loop to push samples to input buffer.
      for (int channel = 0; channel < block.getNumChannels(); channel++) {
        input.pushSample(block.getSample(channel, sample), channel);
        block.setSample(channel, sample, 0.0);

        if (channel == block.getNumChannels() - 1) {
          reqSamples = rubberband->getSamplesRequired();

          if (reqSamples <=
              input.getAvailableSamples(0)) { // Check to trigger rubberband to
                                              // process when full enough.
            readSpace = output.getAvailableSamples(0);

            if (readSpace <
                smallestAcceptableSize) { // Compress or stretch time when
                                          // output ring buffer is too full or
                                          // empty.
              timeSmoothing.setTargetValue(1.1);
            } else if (readSpace > largestAcceptableSize) {
              timeSmoothing.setTargetValue(0.9);
            } else {
              timeSmoothing.setTargetValue(1.0);
            }
            rubberband->setTimeRatio(timeSmoothing.skip((int)reqSamples));
            newPitch = pitchSmoothing.skip((int)reqSamples);
            if (oldPitch != newPitch) {
              rubberband->setPitchScale(newPitch);
              oldPitch = newPitch;
            }
            rubberband->process(input.readPointerArray((int)reqSamples),
                                reqSamples,
                                false); // Process stored input samples.
          }
        }
      }
    }

    auto availableSamples = rubberband->available();

    if (availableSamples > 0) { // If rubberband samples are available then copy
                                // to the output ring buffer.
      rubberband->retrieve(output.writePointerArray(), availableSamples);
      output.copyToBuffer(availableSamples);
    }

    auto availableOutputSamples =
        output.getAvailableSamples(0) -
        samplesToSkip; // Copy samples from output ring buffer to output buffer
                       // where available.
    if (samplesToSkip > 0) {
      int thisSkip = juce::jmin(output.getAvailableSamples(0), samplesToSkip);
      for (int sample = 0; sample < thisSkip; ++sample) {
        for (int channel = 0; channel < block.getNumChannels(); ++channel) {
          output.popSample(channel);
        }
        samplesToSkip--;
      }
    }

    for (int channel = 0; channel < block.getNumChannels(); ++channel) {
      for (int sample = 0; sample < block.getNumSamples(); ++sample) {
        if (output.getAvailableSamples(channel) > 0) {
          block.setSample(
              channel,
              (int)((availableOutputSamples >= block.getNumSamples())
                        ? sample
                        : sample + block.getNumSamples() -
                              availableOutputSamples),
              output.popSample(channel));
        }
      }
    }

    if (pitchParam == 0 &&
        mixParam != 0.0) { // Ensure no phasing with mix occurs when pitch is
                           // set to +/-0 semitones.
      mixSmoothing.setTargetValue(0.0);
    } else {
      mixSmoothing.setTargetValue(mixParam / 100.0);
    }
    dryWet->setWetMixProportion(mixSmoothing.skip((int)block.getNumSamples()));
    dryWet->mixWetSamples(block); // Mix in the dry signal.
  }

  /** Set the wet/dry mix as a % value.
   */
  void setMixPercentage(float newPercentage) { mixParam = newPercentage; }

  /** Set the pitch shift in semitones.
   */
  void setSemitoneShift(float newShift) {
    pitchParam = newShift;

    smallestAcceptableSize = maxSamples * 10.0;
    largestAcceptableSize = maxSamples * 20.0;
  }

  /** Get the % value of the wet/dry mix.
   */
  float getMixPercentage() { return mixParam; }

  /** Get the pitch shift in semitones.
   */
  float getSemitoneShift() { return pitchParam; }

  /** Get the estimated latency. This is an average guess of latency with no
   * pitch shifting but can vary by a few buffers. Changing the pitch shift can
   * cause less or more latency.
   */
  int getLatencyEstimationInSamples() { return maxSamples * 3.0 + initLatency; }

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