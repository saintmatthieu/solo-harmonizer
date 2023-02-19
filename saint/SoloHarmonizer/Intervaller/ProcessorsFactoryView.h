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
  virtual bool hasIntervaller() const = 0;
  virtual std::shared_ptr<Intervaller> getIntervaller() const = 0;
  virtual bool useHostPlayhead() const = 0;
};
} // namespace saint
