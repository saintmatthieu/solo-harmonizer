#include "DefaultMidiFileOwner.h"
#include "CommonTypes.h"
#include "IntervalGetter.h"
#include "IntervalGetterDebugCb.h"
#include "IntervalHelper.h"
#include "JuceMidiFileUtils.h"
#include "PositionGetter.h"
#include "Utils.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>

namespace saint {

DefaultMidiFileOwner::DefaultMidiFileOwner(
    OnCrotchetsPerSecondAvailable onCrotchetsPerSecondAvailable,
    OnPlayheadCommand onPlayheadCommand)
    : _onCrotchetsPerSecondAvailable(onCrotchetsPerSecondAvailable),
      _onPlayheadCommand(onPlayheadCommand) {}

void DefaultMidiFileOwner::setSampleRate(int sampleRate) {
  _samplesPerSecond = sampleRate;
}

void DefaultMidiFileOwner::setStateChangeListener(Listener *listener) {
  assert(_stateChangeListener != listener);
  _stateChangeListener = listener;
}

void DefaultMidiFileOwner::setMidiFile(std::filesystem::path path) {
  _setMidiFile(std::move(path), true);
}

std::optional<std::filesystem::path> DefaultMidiFileOwner::getMidiFile() const {
  return _midiFilePath;
}

std::vector<std::string> DefaultMidiFileOwner::getMidiFileTrackNames() const {
  return _trackNames;
}

void DefaultMidiFileOwner::setPlayedTrack(int track) {
  _setPlayedTrack(track, true);
}

std::optional<int> DefaultMidiFileOwner::getPlayedTrack() const {
  return _playedTrack;
}

void DefaultMidiFileOwner::setHarmonyTrack(int track) {
  _setHarmonyTrack(track, true);
}

std::optional<int> DefaultMidiFileOwner::getHarmonyTrack() const {
  return _harmonyTrack;
}

void DefaultMidiFileOwner::setLoopBeginBar(int bar) { _loopBeginBar = bar; }

std::optional<int> DefaultMidiFileOwner::getLoopBeginBar() const {
  return _loopBeginBar;
}

void DefaultMidiFileOwner::setLoopEndBar(int bar) { _loopEndBar = bar; }

std::optional<int> DefaultMidiFileOwner::getLoopEndBar() const {
  return _loopEndBar;
}

namespace {
void addChildElement(juce::XmlElement &parent, const std::string &name,
                     const std::string &content) {
  auto child = new juce::XmlElement(juce::String{name});
  child->addTextElement(content);
  parent.prependChildElement(child);
}

std::optional<std::string> getChildText(juce::XmlElement &parent,
                                        const std::string &name) {
  if (auto el = parent.getChildByName(name)) {
    return el->getAllSubText().toStdString();
  } else {
    return std::nullopt;
  }
}

std::optional<int> getIntValue(juce::XmlElement &parent,
                               const std::string &childName) {
  if (const auto str = getChildText(parent, childName)) {
    try {
      return std::stoi(*str);
    } catch (...) {
      // TODO handle
      return std::nullopt;
    }
  } else {
    return std::nullopt;
  }
}
} // namespace

std::vector<char> DefaultMidiFileOwner::getState() const {
  juce::XmlElement state{"SoloHarmonizerState"};
  if (_midiFilePath) {
    addChildElement(state, "MidiFile", _midiFilePath->string());
  }
  if (_playedTrack.has_value()) {
    addChildElement(state, "PlayedTrack", std::to_string(*_playedTrack));
  }
  if (_harmonyTrack.has_value()) {
    addChildElement(state, "HarmonyTrack", std::to_string(*_harmonyTrack));
  }
  if (_loopBeginBar.has_value()) {
    addChildElement(state, "LoopBeginBar", std::to_string(*_loopBeginBar));
  }
  if (_loopEndBar.has_value()) {
    addChildElement(state, "LoopEndBar", std::to_string(*_loopEndBar));
  }
  const auto str = state.toString().toStdString();
  std::vector<char> vec(str.size());
  std::copy(str.begin(), str.end(), vec.begin());
  return vec;
}

void DefaultMidiFileOwner::setState(std::vector<char> data) {
  std::string str{data.data()};
  auto newState = juce::parseXML(str);
  if (!newState) {
    // TODO handle
    return;
  }
  auto somethingChanged = false;
  if (const auto pathStr = getChildText(*newState, "MidiFile")) {
    const std::filesystem::path path{*pathStr};
    if (std::filesystem::exists(path)) {
      _setMidiFile(path, false);
      somethingChanged = true;
    } else {
      // TODO handle
    }
  }
  if (const auto playedTrack = getIntValue(*newState, "PlayedTrack")) {
    _setPlayedTrack(*playedTrack, false);
    somethingChanged = true;
  }
  if (const auto harmonyTrack = getIntValue(*newState, "HarmonyTrack")) {
    _setHarmonyTrack(*harmonyTrack, false);
    somethingChanged = true;
  }
  if (const auto loopBeginBar = getIntValue(*newState, "LoopBeginBar")) {
    setLoopBeginBar(*loopBeginBar);
    somethingChanged = true;
  }

  if (const auto loopEndBar = getIntValue(*newState, "LoopEndBar")) {
    setLoopEndBar(*loopEndBar);
    somethingChanged = true;
  }

  if (somethingChanged) {
    if (_stateChangeListener) {
      _stateChangeListener->onStateChange();
    }
  }
}

bool DefaultMidiFileOwner::hasIntervalGetter() const {
  return _intervalGetter.use_count() > 0;
}

std::shared_ptr<IntervalGetter>
DefaultMidiFileOwner::getIntervalGetter() const {
  return _intervalGetter;
}

bool DefaultMidiFileOwner::hasPositionGetter() const {
  return _positionGetter.use_count() > 0;
}

std::shared_ptr<PositionGetter>
DefaultMidiFileOwner::getPositionGetter() const {
  return _positionGetter;
}

namespace {
std::optional<float> getLowestPlayedTrackHarmonizedFrequency(
    const std::vector<IntervalSpan> &spans) {
  std::vector<IntervalSpan> harmonizedSpans;
  std::copy_if(spans.begin(), spans.end(), std::back_inserter(harmonizedSpans),
               [](const IntervalSpan &span) {
                 return span.playedNote.has_value() &&
                        span.playedNote->interval.has_value();
               });
  const auto it = std::min_element(
      harmonizedSpans.begin(), harmonizedSpans.end(),
      [](const IntervalSpan &a, const IntervalSpan &b) -> bool {
        return a.playedNote->noteNumber < b.playedNote->noteNumber;
      });
  if (it == harmonizedSpans.end()) {
    return std::nullopt;
  } else {
    return utils::getPitch(it->playedNote->noteNumber);
  }
}
} // namespace

void DefaultMidiFileOwner::_setMidiFile(
    std::filesystem::path path, bool createIntervalGetterIfAllParametersSet) {
  _juceMidiFile = getJuceMidiFile(path.string());
  _midiFilePath = std::move(path);
  _trackNames = _juceMidiFile ? getTrackNames(*_juceMidiFile)
                              : std::vector<std::string>{};
  _ticksPerCrotchet =
      _juceMidiFile
          ? std::optional<int>{saint::getTicksPerCrotchet(*_juceMidiFile)}
          : std::optional<int>{};
  _crotchetsPerSecond = _juceMidiFile
                            ? extractCrotchetsPerSecond(*_juceMidiFile)
                            : std::optional<float>{};
  if (_crotchetsPerSecond.has_value()) {
    _onCrotchetsPerSecondAvailable(*_crotchetsPerSecond);
  }
  if (_juceMidiFile && _ticksPerCrotchet) {
    auto timeSignaturePositions = getTimeSignatures(*_juceMidiFile);
    _positionGetter =
        std::make_shared<PositionGetter>(std::move(timeSignaturePositions));
  }
  if (createIntervalGetterIfAllParametersSet) {
    _createIntervalGetterIfAllParametersSet();
  }
}

void DefaultMidiFileOwner::_setPlayedTrack(
    int track, bool createIntervalGetterIfAllParametersSet) {
  if (_playedTrack != track) {
    _playedTrack = track;
    if (createIntervalGetterIfAllParametersSet) {
      _createIntervalGetterIfAllParametersSet();
    }
  }
}

void DefaultMidiFileOwner::_setHarmonyTrack(
    int track, bool createIntervalGetterIfAllParametersSet) {
  if (_harmonyTrack != track) {
    _harmonyTrack = track;
    if (createIntervalGetterIfAllParametersSet) {
      _createIntervalGetterIfAllParametersSet();
    }
  }
}

void DefaultMidiFileOwner::_createIntervalGetterIfAllParametersSet() {
  if (!_juceMidiFile || !_ticksPerCrotchet || !_playedTrack || !_harmonyTrack ||
      !_samplesPerSecond || !_crotchetsPerSecond) {
    return;
  }
  const auto playedSeq = getMidiNoteMessages(
      *_juceMidiFile->getTrack(*_playedTrack), *_ticksPerCrotchet);
  const auto harmoSeq = getMidiNoteMessages(
      *_juceMidiFile->getTrack(*_harmonyTrack), *_ticksPerCrotchet);
  _intervalGetterInput = toIntervalSpans(playedSeq, harmoSeq);
  _lowestPlayedTrackHarmonizedFrequency =
      ::saint::getLowestPlayedTrackHarmonizedFrequency(_intervalGetterInput);
  if (_intervalGetterInput.empty()) {
    // _logger->warn("toIntervalGetterInput returned empty vector");
  } else {
    const auto ticksPerSample =
        *_ticksPerCrotchet * *_crotchetsPerSecond / *_samplesPerSecond;
    // TODO: No need to wrap IntervalGetter
    if (utils::getEnvironmentVariableAsBool("SAINT_DEBUG_INTERVALGETTER") &&
        utils::isDebugBuild()) {
      _intervalGetter = std::make_shared<IntervalGetter>(
          _intervalGetterInput, *_ticksPerCrotchet, ticksPerSample,
          testUtils::getIntervalGetterDebugCb());
    } else {
      _intervalGetter = std::make_shared<IntervalGetter>(
          _intervalGetterInput, *_ticksPerCrotchet, ticksPerSample,
          std::nullopt);
    }
  }
}

std::optional<int> DefaultMidiFileOwner::getTicksPerCrotchet() const {
  return _ticksPerCrotchet;
}

std::optional<float> DefaultMidiFileOwner::getCrotchetsPerSecond() const {
  return _crotchetsPerSecond;
}

std::optional<float>
DefaultMidiFileOwner::getLowestPlayedTrackHarmonizedFrequency() const {
  return _lowestPlayedTrackHarmonizedFrequency;
}

bool DefaultMidiFileOwner::execute(PlayheadCommand command) {
  return _onPlayheadCommand(command);
}

} // namespace saint
