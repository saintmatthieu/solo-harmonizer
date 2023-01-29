#pragma once

#include <fstream>
#include <sbsms.h>
#include <vector>

namespace sbsms = _sbsms_;

class SbsmsWrapper : public sbsms::SBSMSInterface {
public:
  SbsmsWrapper(std::vector<float> input);
  void get(float *audio, long n);
  size_t getInputFrameSize();
  void setPitchShift(float value);
  float getPitchShift() const;

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
  const std::vector<float> _input;
  sbsms::SBSMSQuality _quality;
  sbsms::SBSMS _sbsms;
  float _pitchShift = 1.f;
  size_t _numSamplesRead = 0u;
  std::ofstream _log;
};
