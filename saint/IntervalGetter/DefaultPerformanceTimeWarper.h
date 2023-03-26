#include "PerformanceTimeWarper.h"

#include <vector>

namespace saint {
class DefaultPerformanceTimeWarper : public PerformanceTimeWarper {
public:
  DefaultPerformanceTimeWarper(
      const std::map<float, std::optional<int>> &timedNoteNumbers);
  float getWarpedTime(float timeInCrotchets,
                      const std::optional<float> &pitch) override;

private:
  const std::vector<float> _noteBeginCrotchets;
  int _lastIndex = 0;
};
} // namespace saint
