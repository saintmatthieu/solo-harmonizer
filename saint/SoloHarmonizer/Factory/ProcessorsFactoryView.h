#pragma once

#include "HarmoPitchGetter.h"

#include <filesystem>
#include <optional>

namespace saint {
class ProcessorsFactoryView {
public:
  virtual ~ProcessorsFactoryView() = default;
  virtual const std::vector<uint8_t> &getState() const = 0;
  virtual void setState(std::vector<uint8_t>) = 0;
  virtual bool hasHarmoPitchGetter() const = 0;
  virtual std::shared_ptr<HarmoPitchGetter> getHarmoPitchGetter() const = 0;
  virtual bool useHostPlayhead() const = 0;
};
} // namespace saint
