#pragma once

#include "EditorsFactoryView.h"
#include "HarmoPitchTypes.h"
#include "ProcessorsFactoryView.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace saint {
class IntervallerFactory : public EditorsFactoryView,
                           public ProcessorsFactoryView {
public:
  // EditorsFactoryView
  void setMidiFile(std::filesystem::path) override;
  std::optional<std::filesystem::path> getMidiFile() const override;
  std::vector<std::string> getMidiFileTrackNames() const override;
  void setPlayedTrack(int) override;
  std::optional<int> getPlayedTrack() const override;
  void setHarmonyTrack(int) override;
  std::optional<int> getHarmonyTrack() const override;

  // ProcessorsFactoryView
  const std::vector<uint8_t> &getState() const override;
  void setState(std::vector<uint8_t>) override;
  std::unique_ptr<Intervaller> prepareToPlay() const override;

private:
  void _prepareHarmoPitchGetterInputIfAllParametersSet();
  std::vector<HarmoNoteSpan> _harmoPitchGetterInput;
  std::optional<juce::MidiFile> _juceMidiFile;
  std::optional<std::filesystem::path> _midiFilePath;
  std::vector<std::string> _trackNames;
  std::optional<int> _playedTrack;
  std::optional<int> _harmonyTrack;
  std::optional<int> _ticksPerCrotchet;
  std::optional<float> _crotchetsPerSecond;
  std::vector<uint8_t> _state;
};
} // namespace saint
