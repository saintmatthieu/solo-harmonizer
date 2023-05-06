#pragma once

#include <optional>
#include <utility>
#include <vector>

namespace saint {
class MelodyRecognizer2 {
public:
  using Melody =
      std::vector<std::pair<float /*duration*/, int /*note number*/>>;
  MelodyRecognizer2(Melody melody);
  std::optional<size_t> onNoteOff(const std::vector<float> &noteNumbers);

private:
  const Melody _melody;
  const std::vector<int> _noteNumbers;
  const std::vector<float> _durations;
  std::vector<std::vector<float>> _lastExperiments;
};
} // namespace saint
