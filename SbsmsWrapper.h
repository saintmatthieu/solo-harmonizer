#pragma once

#include "Ring-Buffer/ringbuffer.hpp"
#include <sbsms.h>

namespace sbsms = _sbsms_;

class SbsmsWrapper : public sbsms::SBSMSInterface {
public:
  SbsmsWrapper();
  void process(float *audio, long n);
  size_t getInputFrameSize();

private:
  // sbsms::SBSMSInterface
  long samples(sbsms::audio *buf, long n) override;
  float getStretch(float t) override;
  float getMeanStretch(float t0, float t1) override;
  float getPitch(float t) override;
  long getPresamples() override;
  sbsms::SampleCountType getSamplesToInput() override;
  sbsms::SampleCountType getSamplesToOutput() override;

private:
  jnk0le::Ringbuffer<float, 1024> _ringBuffer;
  sbsms::SBSMSQuality _quality;
  sbsms::SBSMS _sbsms;
  // sbsms::Resampler _resampler;
};
