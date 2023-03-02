#pragma once

#include "DavidCNAntonia/IPitchShifter.h"
#include "Factory/ProcessorsFactoryView.h"
#include "PitchDetector.h"
#include "Playheads/IPlayhead.h"

#include <spdlog/spdlog.h>

namespace saint {
class SoloHarmonizer {
public:
  SoloHarmonizer(std::shared_ptr<ProcessorsFactoryView>, IPlayhead &);
  ~SoloHarmonizer();

  void setSemitoneShift(float value);
  void prepareToPlay(double sampleRate, int samplesPerBlock);
  void processBlock(float *, int size);
  std::vector<uint8_t> getState() const;
  void setState(std::vector<uint8_t>);
  void releaseResources();

private:
  const std::shared_ptr<ProcessorsFactoryView> _processorsFactoryView;
  const std::string _loggerName;
  const std::shared_ptr<spdlog::logger> _logger;
  IPlayhead &_playhead;
  std::unique_ptr<DavidCNAntonia::IPitchShifter> _pitchShifter;
  std::unique_ptr<PitchDetector> _pitchDetector;
  std::optional<float> _pitchShift;
};
} // namespace saint
