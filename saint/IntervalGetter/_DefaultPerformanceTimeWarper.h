#include "PerformanceTimeWarper.h"

#include <queue>
#include <vector>

namespace saint {
class DefaultPerformanceTimeWarper : public PerformanceTimeWarper {
public:
  DefaultPerformanceTimeWarper(
      const std::vector<std::pair<float, std::optional<int>>>
          &midiFileNoteNumbers);
  float getWarpedTime(float playheadTime,
                      const std::optional<float> &noteNumber) override;

private:
  void _updateMeasurements(float playheadTime, const std::optional<float> &nn);
  std::vector<int> _getHypothesisEndIndices(float playheadTime);
  void
  _updateLastStateChangeMatchTime(float playheadTime,
                                  const std::vector<int> &hypothesisEndIndices);
  const std::vector<std::pair<float, std::optional<int>>> _fileBreakpoints;
  std::deque<float> _measurementTimes;
  std::deque<std::optional<float>> _measuredNns;
  bool _prevWasPitched = false;
  float _playheadTimeByLastChange = 0.f;
  float _lastStateChangeMatchTime = 0.f;
};
} // namespace saint
