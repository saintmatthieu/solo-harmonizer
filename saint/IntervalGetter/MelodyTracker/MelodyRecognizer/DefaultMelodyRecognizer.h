#pragma once

#include "MelodyTracker/MelodyRecognizer/MelodyRecognizer.h"
#include "ObservationLikelihoodGetter/ObservationLikelihoodGetter.h"
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace saint {
class DefaultMelodyRecognizer : public MelodyRecognizer {
public:
  DefaultMelodyRecognizer(
      std::unique_ptr<ObservationLikelihoodGetter>,
      const std::vector<std::pair<float, std::optional<int>>> &melody);

  bool onNoteOff(const std::vector<std::pair<std::chrono::milliseconds, float>>
                     &) override;
  size_t getNextNoteIndex() override;

private:
  const std::unique_ptr<ObservationLikelihoodGetter> _likelihoodGetter;
  const std::vector<int> _melody;
  const std::vector<int> _intervals;
  const std::vector<std::set<std::vector<int>>> _uniqueIntervals;
  std::optional<size_t> _nextNoteIndex;
};
} // namespace saint
