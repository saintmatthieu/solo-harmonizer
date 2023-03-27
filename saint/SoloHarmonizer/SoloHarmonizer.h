#pragma once

#include "DavidCNAntonia/IPitchShifter.h"
#include "MidiFileOwner.h"
#include "PitchDetector.h"
#include "Playhead.h"

#include <spdlog/spdlog.h>

namespace saint {
class SoloHarmonizer : public MidiFileOwner::Listener {
public:
  SoloHarmonizer(std::shared_ptr<MidiFileOwner>, Playhead &);
  ~SoloHarmonizer();

  void setSemitoneShift(float value);
  void prepareToPlay(int sampleRate, int samplesPerBlock);
  void processBlock(float *, int size);
  void releaseResources();

private:
  void onCrotchetsPerSecondAvailable(float) override;
  void _updateTestCbsIfReady();

  const std::shared_ptr<MidiFileOwner> _midiFileOwner;
  const std::string _loggerName;
  const std::shared_ptr<spdlog::logger> _logger;
  Playhead &_playhead;
  std::unique_ptr<DavidCNAntonia::IPitchShifter> _pitchShifter;
  std::unique_ptr<PitchDetector> _pitchDetector;
  std::optional<float> _pitchShift;
  int _samplesPerBlock = 0;
  float _crotchetsPerSecond = 0;
};
} // namespace saint
