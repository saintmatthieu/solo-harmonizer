#pragma once

#include "SoloHarmonizerProcessor.h"

//==============================================================================
class SoloHarmonizerEditor : public juce::AudioProcessorEditor {
public:
  explicit SoloHarmonizerEditor(SoloHarmonizerProcessor &);
  ~SoloHarmonizerEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  SoloHarmonizerProcessor &processorRef;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoloHarmonizerEditor)
};
