#pragma once

#include "CommonTypes.h"
#include "IntervalGetter.h"
#include "MelodyRecognizer3/MelodyRecognizer3.h"
#include "PitchMapper.h"

namespace saint {
class StochasticIntervalGetter : public IntervalGetter {
public:
  StochasticIntervalGetter(const std::vector<IntervalSpan> &spans,
                           std::unique_ptr<MelodyRecognizer3>,
                           const std::map<float, Fraction> &timeSignatures);
  std::optional<float>
  getHarmoInterval(float timeInCrotchets,
                   const std::optional<std::function<float(int)>> &getPitchLlh,
                   const std::chrono::milliseconds &now,
                   std::optional<size_t> &melodyRecognizerDebugOut,
                   int blockSize = 0) override;

private:
  const std::vector<IntervalSpan> _spans;
  const std::unique_ptr<MelodyRecognizer3> _melodyRecognizer;
  const std::unique_ptr<PitchMapper> _pitchMapper;
  bool _prevPitchHadValue = false;
  std::optional<float> _nextNoteonTimeEstimate;
  int _tick = 0;
};
} // namespace saint
