#pragma once

#include "IntervalGetter.h"
#include "SoloHarmonizerTypes.h"

#include <filesystem>
#include <optional>

namespace saint {
class MidiFileOwner {
public:
  virtual ~MidiFileOwner() = default;
  virtual void setMidiFile(std::filesystem::path) = 0;
  virtual std::optional<std::filesystem::path> getMidiFile() const = 0;
  virtual std::vector<std::string> getMidiFileTrackNames() const = 0;
  virtual void setPlayedTrack(int) = 0;
  virtual std::optional<int> getPlayedTrack() const = 0;
  virtual void setHarmonyTrack(int) = 0;
  virtual std::optional<int> getHarmonyTrack() const = 0;
  virtual bool execute(PlayheadCommand) = 0;
  virtual const std::vector<uint8_t> &getState() const = 0;
  virtual void setState(std::vector<uint8_t>) = 0;
  virtual bool hasIntervalGetter() const = 0;
  virtual std::shared_ptr<IntervalGetter> getIntervalGetter() const = 0;
  virtual std::optional<float>
  getLowestPlayedTrackHarmonizedFrequency() const = 0;
};
} // namespace saint
