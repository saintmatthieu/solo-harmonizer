#include "SoloHarmonizer.h"
#include "IntervalGetter.h"
#include "MidiFileOwner.h"
#include "SoloHarmonizerHelper.h"

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace saint {
namespace {
static std::atomic<int> instanceCounter = 0;
} // namespace

SoloHarmonizer::SoloHarmonizer(std::shared_ptr<MidiFileOwner> midiFileOwner,
                               Playhead &playhead)
    : _midiFileOwner(std::move(midiFileOwner)),
      _loggerName(std::string{"SoloHarmonizer_"} +
                  std::to_string(instanceCounter++)),
      _logger(spdlog::basic_logger_mt(
          _loggerName, saint::generateLogFilename(_loggerName).string())),
      _playhead(playhead) {
  _logger->set_level(saint::getLogLevelFromEnv());
  _logger->info("ctor {0}", _loggerName);
  _midiFileOwner->addStateChangeListener(this);
}

SoloHarmonizer::~SoloHarmonizer() {
  _logger->info("dtor {0}", _loggerName);
  // Not necessary, but sure ...
  _midiFileOwner->removeStateChangeListener(this);
}

void SoloHarmonizer::prepareToPlay(int sampleRate, int samplesPerBlock) {
  _pitchShifter = DavidCNAntonia::IPitchShifter::createInstance(
      1, static_cast<double>(sampleRate), samplesPerBlock);
  _pitchDetector = PitchDetector::createInstance(
      sampleRate, _midiFileOwner->getLowestPlayedTrackHarmonizedFrequency());
  _samplesPerBlock = samplesPerBlock;
  _logger->info("prepareToPlay sampleRate={0} samplesPerBlock={1}", sampleRate,
                samplesPerBlock);
}

void SoloHarmonizer::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
  _pitchShifter.reset();
  _logger->info("releaseResources");
  _logger->flush();
}

void SoloHarmonizer::setSemitoneShift(float value) {
  _pitchShifter->setSemitoneShift(value);
}

void SoloHarmonizer::processBlock(const std::chrono::milliseconds &now,
                                  float *block, int size) {
  _logger->trace("processBlock");
  if (!_midiFileOwner->hasIntervalGetter()) {
    return;
  }
  const auto intervalGetter = _midiFileOwner->getIntervalGetter();
  if (!intervalGetter) {
    return;
  }
  const auto timeOpt = _playhead.getTimeInCrotchets();
  if (!timeOpt.has_value()) {
    // TODO logging
    return;
  }
  const auto time = *timeOpt;
  const auto getPitchLlh = _pitchDetector->process(block, size);
  const auto pitchShift =
      intervalGetter->getHarmoInterval(time, getPitchLlh, now, size);
  _logger->debug("_intervalGetter->getHarmoInterval() returned {0}",
                 pitchShift ? std::to_string(*pitchShift) : "nullopt");
  if (pitchShift.has_value()) {
    _pitchShifter->setMixPercentage(50.f);
    _pitchShifter->setSemitoneShift(*pitchShift);
  } else {
    _pitchShifter->setMixPercentage(0.f);
  }
  std::vector<float *> channels(1);
  channels[0] = block;
  _pitchShifter->processBuffer(channels.data(), 1, size);
}

void SoloHarmonizer::onCrotchetsPerSecondAvailable(float crotchetsPerSecond) {
  _crotchetsPerSecond = crotchetsPerSecond;
}
} // namespace saint
