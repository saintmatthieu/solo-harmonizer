#include "SbsmsWrapper.h"
#include "sbsms.h"

#include <array>
#include <functional>
#include <memory>

SbsmsWrapper::SbsmsWrapper(std::vector<float> input)
    : _input(std::move(input)), _quality(&sbsms::SBSMSQualityStandard),
      _sbsms(1, &_quality, true /* on va dire true */),
      _log("C:/Users/saint/Downloads/log.txt") {}

void SbsmsWrapper::get(float *audio, long n) {
  const auto s = (size_t)n;
  std::array<float[2], 1024> d;
  _log << "push " << n << std::endl;
  _sbsms.read(this, d.data(), n);
  for (auto i = 0u; i < s; ++i) {
    audio[i] = d[i][0];
  }
}

long SbsmsWrapper::samples(sbsms::audio *buf, long n) {
  _log << "pull " << n << std::endl;
  const auto numSamplesToRead =
      std::min(_input.size() - _numSamplesRead, (size_t)n);
  for (auto i = 0u; i < numSamplesToRead; ++i) {
    buf[i][0] = buf[i][1] = _input[_numSamplesRead + i];
  }
  _numSamplesRead += numSamplesToRead;
  return (long)numSamplesToRead;
}

void SbsmsWrapper::setPitchShift(float value) { _pitchShift = value; }

float SbsmsWrapper::getPitchShift() const { return _pitchShift; }

float SbsmsWrapper::getStretch(float t) { return 1.f; }

float SbsmsWrapper::getMeanStretch(float t0, float t1) { return 1.f; }

float SbsmsWrapper::getPitch(float t) { return _pitchShift; }

long SbsmsWrapper::getPresamples() {
  // Whatever that is, seems relevant only to non-constant mode ...
  return 0;
}

sbsms::SampleCountType SbsmsWrapper::getSamplesToInput() {
  // Whatever that is, seems relevant only to non-constant mode ...
  return 0;
}

sbsms::SampleCountType SbsmsWrapper::getSamplesToOutput() {
  // May be fine as long as the pitch shift is constant ...
  return 0;
}

size_t SbsmsWrapper::getInputFrameSize() {
  return (size_t)_sbsms.getInputFrameSize();
}
