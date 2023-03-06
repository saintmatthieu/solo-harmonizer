#pragma once

#include "IntervalGetter.h"

#include <filesystem>
#include <optional>

namespace saint {
class ProcessorsFactoryView {
public:
  virtual ~ProcessorsFactoryView() = default;
  virtual const std::vector<uint8_t> &getState() const = 0;
  virtual void setState(std::vector<uint8_t>) = 0;
  virtual bool hasIntervalGetter() const = 0;
  virtual std::shared_ptr<IntervalGetter> getIntervalGetter() const = 0;
  virtual bool useHostPlayhead() const = 0;
  virtual std::optional<float>
  getLowestPlayedTrackHarmonizedFrequency() const = 0;
};
} // namespace saint
