#pragma once

#include "EditorsFactoryView.h"
#include "HarmoPitchGetter.h"
#include "HarmoPitchTypes.h"
#include "ProcessorsFactoryView.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace saint {
class HarmoPitchGetterFactory : public EditorsFactoryView,
                                public ProcessorsFactoryView {
public:
  // EditorsFactoryView
  void setUseHostPlayhead(bool) override;
  bool getUseHostPlayhead() const override;
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
  bool hasHarmoPitchGetter() const override;
  std::shared_ptr<HarmoPitchGetter> getHarmoPitchGetter() const override;
  bool useHostPlayhead() const override;

private:
  void _createHarmoPitchGetterIfAllParametersSet();
  std::vector<HarmoNoteSpan> _harmoPitchGetterInput;
  std::optional<juce::MidiFile> _juceMidiFile;
  std::optional<std::filesystem::path> _midiFilePath;
  std::vector<std::string> _trackNames;
  std::optional<int> _playedTrack;
  std::optional<int> _harmonyTrack;
  std::optional<int> _ticksPerCrotchet;
  std::optional<float> _crotchetsPerSecond;
  std::vector<uint8_t> _state;
  std::shared_ptr<HarmoPitchGetter> _harmoPitchGetter;
  bool _useHostPlayhead = true;
};
} // namespace saint
