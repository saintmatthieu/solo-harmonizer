#include "DefaultMidiFileOwner.h"
#include "CommonTypes.h"
#include "IntervalGetter.h"
#include "IntervalGetterDebugCb.h"
#include "IntervalHelper.h"
#include "JuceMidiFileUtils.h"
#include "PositionGetter.h"
#include "Utils.h"

#include <algorithm>
#include <iterator>

namespace saint {

DefaultMidiFileOwner::DefaultMidiFileOwner(
    OnCrotchetsPerSecondAvailable onCrotchetsPerSecondAvailable,
    OnPlayheadCommand onPlayheadCommand)
    : _onCrotchetsPerSecondAvailable(onCrotchetsPerSecondAvailable),
      _onPlayheadCommand(onPlayheadCommand) {}

void DefaultMidiFileOwner::setSampleRate(int sampleRate) {
  _samplesPerSecond = sampleRate;
}

void DefaultMidiFileOwner::setMidiFile(std::filesystem::path path) {
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
  _createIntervalGetterIfAllParametersSet();
}

std::optional<std::filesystem::path> DefaultMidiFileOwner::getMidiFile() const {
  return _midiFilePath;
}

std::vector<std::string> DefaultMidiFileOwner::getMidiFileTrackNames() const {
  return _trackNames;
}

void DefaultMidiFileOwner::setPlayedTrack(int track) {
  if (_playedTrack != track) {
    _playedTrack = track;
    _createIntervalGetterIfAllParametersSet();
  }
}

std::optional<int> DefaultMidiFileOwner::getPlayedTrack() const {
  return _playedTrack;
}

void DefaultMidiFileOwner::setHarmonyTrack(int track) {
  if (_harmonyTrack != track) {
    _harmonyTrack = track;
    _createIntervalGetterIfAllParametersSet();
  }
}

std::optional<int> DefaultMidiFileOwner::getHarmonyTrack() const {
  return _harmonyTrack;
}

const std::vector<uint8_t> &DefaultMidiFileOwner::getState() const {
  return _state;
}

void DefaultMidiFileOwner::setState(std::vector<uint8_t>) {}

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
