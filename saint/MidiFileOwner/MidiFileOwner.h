#pragma once

#include "CommonTypes.h"

#include <filesystem>
#include <optional>

namespace saint {
class MidiFileOwner {
public:
  class Listener {
  public:
    virtual ~Listener() = default;
    virtual void onStateChange() {}
    virtual void onLoopBeginBarChange(const std::optional<int> &) {}
    virtual void onLoopEndBarChange(const std::optional<int> &) {}
    virtual void onIntervalSpansAvailable(const std::vector<IntervalSpan> &) {}
  };

  virtual ~MidiFileOwner() = default;
  virtual void setSampleRate(int) = 0; // This is needed for a debug feature. A
                                       // better could probably be found.
  virtual void addStateChangeListener(Listener *) = 0;
  virtual void removeStateChangeListener(Listener *) = 0;
  virtual void setMidiFile(std::filesystem::path) = 0;
  virtual std::optional<std::filesystem::path> getMidiFile() const = 0;
  virtual std::vector<std::string> getMidiFileTrackNames() const = 0;
  virtual void setPlayedTrack(int) = 0;
  virtual std::optional<int> getPlayedTrack() const = 0;
  virtual void setHarmonyTrack(int) = 0;
  virtual std::optional<int> getHarmonyTrack() const = 0;
  virtual void setLoopBeginBar(std::optional<int>) = 0;
  virtual std::optional<int> getLoopBeginBar() const = 0;
  virtual void setLoopEndBar(std::optional<int>) = 0;
  virtual std::optional<int> getLoopEndBar() const = 0;
  virtual bool execute(PlayheadCommand) = 0;
  virtual std::vector<char> getState() const = 0;
  virtual void setState(std::vector<char>) = 0;
  virtual bool hasIntervalGetter() const = 0;
  virtual std::shared_ptr<IntervalGetter> getIntervalGetter() const = 0;
  virtual bool hasPositionGetter() const = 0;
  virtual std::shared_ptr<PositionGetter> getPositionGetter() const = 0;
  virtual std::optional<float>
  getLowestPlayedTrackHarmonizedFrequency() const = 0;
  virtual std::optional<std::vector<IntervalSpan>> getIntervalSpans() const = 0;
};
} // namespace saint
