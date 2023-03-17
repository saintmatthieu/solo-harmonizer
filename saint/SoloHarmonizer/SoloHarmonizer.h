#pragma once

#include "DavidCNAntonia/IPitchShifter.h"
#include "MidiFileOwner.h"
#include "PitchDetector.h"
#include "Playhead.h"

#include <spdlog/spdlog.h>

namespace saint {
class SoloHarmonizer {
public:
  SoloHarmonizer(std::shared_ptr<MidiFileOwner>, Playhead &);
  ~SoloHarmonizer();

  void setSemitoneShift(float value);
  void prepareToPlay(int sampleRate, int samplesPerBlock);
  void processBlock(float *, int size);
  void releaseResources();

private:
  const std::shared_ptr<MidiFileOwner> _midiFileOwner;
  const std::string _loggerName;
  const std::shared_ptr<spdlog::logger> _logger;
  Playhead &_playhead;
  std::unique_ptr<DavidCNAntonia::IPitchShifter> _pitchShifter;
  std::unique_ptr<PitchDetector> _pitchDetector;
  std::optional<float> _pitchShift;
};
} // namespace saint
