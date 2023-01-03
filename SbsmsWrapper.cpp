#include "SbsmsWrapper.h"
#include "sbsms.h"

#include <array>
#include <functional>
#include <memory>

SbsmsWrapper::SbsmsWrapper()
    : _quality(&sbsms::SBSMSQualityStandard),
      _sbsms(1, &_quality, true /* on va dire true */) {}

void SbsmsWrapper::process(float *audio, long n) {
  const auto s = (size_t)n;
  _ringBuffer.writeBuff(audio, s);
  std::array<float[2], 1024> d;
  _sbsms.read(this, d.data(), n);
  for (auto i = 0u; i < s; ++i) {
    audio[i] = d[i][0];
  }
}

long SbsmsWrapper::samples(sbsms::audio *buf, long n) {
  if (_ringBuffer.readAvailable() < n) {
    return 0;
  }
  for (auto i = 0; i < n; ++i) {
    buf[i][0] = buf[i][1] = *_ringBuffer.at((size_t)i);
  }
  return n;
}

float SbsmsWrapper::getStretch(float t) { return 1.f; }

float SbsmsWrapper::getMeanStretch(float t0, float t1) { return 1.f; }

float SbsmsWrapper::getPitch(float t) { return 1.5f; }

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
