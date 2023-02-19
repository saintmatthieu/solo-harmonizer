#pragma once

#include "Intervaller.h"

#include <filesystem>
#include <optional>

namespace saint {
class ProcessorsFactoryView {
public:
  virtual ~ProcessorsFactoryView() = default;
  virtual const std::vector<uint8_t> &getState() const = 0;
  virtual void setState(std::vector<uint8_t>) = 0;
  virtual std::unique_ptr<Intervaller> prepareToPlay() const = 0;
  virtual bool useHostPlayhead() const = 0;
};
} // namespace saint
