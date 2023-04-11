#include "TracingMelodyTracker.h"

namespace saint {
TracingMelodyTracker::TracingMelodyTracker(std::unique_ptr<MelodyTracker> impl)
    : _impl(std::move(impl)),
      _trace("C:\\Users\\saint\\Downloads\\MelodyTracker.txt") {}

void TracingMelodyTracker::onHostTimeJump(float newTime) {
  _trace << "onHostTimeJump," << newTime << std::endl;
  _impl->onHostTimeJump(newTime);
}

size_t
TracingMelodyTracker::onNoteOnSample(const std::chrono::milliseconds &now,
                                     float noteNum) {
  _trace << "onNoteOnSample," << now.count() << "," << noteNum << std::endl;
  return _impl->onNoteOnSample(now, noteNum);
}

void TracingMelodyTracker::onNoteOff() {
  _trace << "onNoteOff" << std::endl;
  _impl->onNoteOff();
}
} // namespace saint