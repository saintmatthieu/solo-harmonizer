#include "PitchShifter.h"

namespace DavidCNAntonia {
PitchShifter::PitchShifter(
    int numChannels, double sampleRate, int samplesPerBlock,
    std::optional<RubberBand::RubberBandStretcher::Options> opts) {
  rubberband = std::make_unique<RubberBand::RubberBandStretcher>(
      sampleRate, numChannels, opts ? *opts : defaultOptions, 1.0, 1.0);

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

void PitchShifter::setFormantPreserving(bool shouldPreserveFormants) {
  if (shouldPreserveFormants != formantPreserving) {
    formantPreserving = shouldPreserveFormants;
    rubberband->setFormantOption(
        (shouldPreserveFormants)
            ? RubberBand::RubberBandStretcher::Option::OptionFormantPreserved
            : RubberBand::RubberBandStretcher::Option::OptionFormantShifted);
  }
}

int PitchShifter::getLatency() { return latencyInSamples; }

void PitchShifter::processBuffer(juce::dsp::AudioBlock<float> &block) {

  pitchSmoothing.setTargetValue(powf(
      2.0, pitchParam / 12)); // Convert semitone value into pitch scale value.

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
        block.setSample(channel,
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

void PitchShifter::setMixPercentage(float newPercentage) {
  mixParam = newPercentage;
}

void PitchShifter::setSemitoneShift(float newShift) {
  pitchParam = newShift;

  smallestAcceptableSize = maxSamples * 10.0;
  largestAcceptableSize = maxSamples * 20.0;
}

float PitchShifter::getMixPercentage() { return mixParam; }

float PitchShifter::getSemitoneShift() { return pitchParam; }

int PitchShifter::getLatencyEstimationInSamples() {
  return maxSamples * 3.0 + initLatency;
}
} // namespace DavidCNAntonia
