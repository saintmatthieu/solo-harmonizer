#pragma once

#include "CommonTypes.h"
#include "MidiFileOwner.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <functional>

namespace saint {

using OnCrotchetsPerSecondAvailable = std::function<void(float)>;
using OnPlayheadCommand = std::function<bool(PlayheadCommand)>;

class DefaultMidiFileOwner : public MidiFileOwner {
public:
  DefaultMidiFileOwner(OnCrotchetsPerSecondAvailable, OnPlayheadCommand);
  void setSampleRate(int);

  // MidiFileOwner
  void setMidiFile(std::filesystem::path) override;
  std::optional<std::filesystem::path> getMidiFile() const override;
  std::vector<std::string> getMidiFileTrackNames() const override;
  void setPlayedTrack(int) override;
  std::optional<int> getPlayedTrack() const override;
  void setHarmonyTrack(int) override;
  std::optional<int> getHarmonyTrack() const override;
  std::optional<float> getLowestPlayedTrackHarmonizedFrequency() const override;
  bool execute(PlayheadCommand) override;
  const std::vector<uint8_t> &getState() const override;
  void setState(std::vector<uint8_t>) override;
  bool hasIntervalGetter() const override;
  std::shared_ptr<IntervalGetter> getIntervalGetter() const override;
  bool hasPositionGetter() const override;
  std::shared_ptr<PositionGetter> getPositionGetter() const override;

  // For testing
  std::optional<int> getTicksPerCrotchet() const;
  std::optional<float> getCrotchetsPerSecond() const;

private:
  void _createIntervalGetterIfAllParametersSet();
  const OnCrotchetsPerSecondAvailable _onCrotchetsPerSecondAvailable;
  const OnPlayheadCommand _onPlayheadCommand;
  std::vector<IntervalSpan> _intervalGetterInput;
  std::optional<juce::MidiFile> _juceMidiFile;
  std::optional<std::filesystem::path> _midiFilePath;
  std::vector<std::string> _trackNames;
  std::optional<int> _samplesPerSecond;
  std::optional<int> _playedTrack;
  std::optional<int> _harmonyTrack;
  std::optional<int> _ticksPerCrotchet;
  std::optional<float> _crotchetsPerSecond;
  std::optional<float> _lowestPlayedTrackHarmonizedFrequency;
  std::vector<uint8_t> _state;
  std::shared_ptr<IntervalGetter> _intervalGetter;
  std::shared_ptr<PositionGetter> _positionGetter;
};
} // namespace saint
