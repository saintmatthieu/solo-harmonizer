#include "OnsetDetector.h"

#include <math.h>

namespace saint {
namespace {
int getFftOrder(int blockSize) {
  return static_cast<int>(ceil(log2(blockSize)));
}
} // namespace
OnsetDetector::OnsetDetector(int samplesRate, int blockSize)
    : _fft(getFftOrder(blockSize)) {}

bool OnsetDetector::process(const float *, int) { return false; }
} // namespace saint
