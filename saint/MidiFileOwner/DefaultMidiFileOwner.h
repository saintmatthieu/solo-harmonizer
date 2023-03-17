#pragma once

#include "CommonTypes.h"
#include "MidiFileOwner.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <functional>

namespace saint {

using OnCrotchetsPerSecondAvailable = std::function<void(float)>;
using OnPlayheadCommand = std::function<bool(PlayheadCommand)>;

class DefaultMidiFileOwner : public MidiFileOwner {
public:
  DefaultMidiFileOwner(OnCrotchetsPerSecondAvailable, OnPlayheadCommand);
  void setSampleRate(int);

  // MidiFileOwner
  void setStateChangeListener(Listener *) override;
  void setMidiFile(std::filesystem::path) override;
  std::optional<std::filesystem::path> getMidiFile() const override;
  std::vector<std::string> getMidiFileTrackNames() const override;
  void setPlayedTrack(int) override;
  std::optional<int> getPlayedTrack() const override;
  void setHarmonyTrack(int) override;
  std::optional<int> getHarmonyTrack() const override;
  std::optional<float> getLowestPlayedTrackHarmonizedFrequency() const override;
  bool execute(PlayheadCommand) override;
  std::vector<char> getState() const override;
  void setState(std::vector<char>) override;
  bool hasIntervalGetter() const override;
  std::shared_ptr<IntervalGetter> getIntervalGetter() const override;
  bool hasPositionGetter() const override;
  std::shared_ptr<PositionGetter> getPositionGetter() const override;

  // For testing
  std::optional<int> getTicksPerCrotchet() const;
  std::optional<float> getCrotchetsPerSecond() const;

private:
  void _setMidiFile(std::filesystem::path,
                    bool createIntervalGetterIfAllParametersSet);
  void _setPlayedTrack(int, bool createIntervalGetterIfAllParametersSet);
  void _setHarmonyTrack(int, bool createIntervalGetterIfAllParametersSet);
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
  std::shared_ptr<IntervalGetter> _intervalGetter;
  std::shared_ptr<PositionGetter> _positionGetter;
  Listener *_stateChangeListener = nullptr;
};
} // namespace saint
