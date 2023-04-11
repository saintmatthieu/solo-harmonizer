#include "DefaultTimingEstimator.h"

#include "gmtl/Matrix.h"
#include "gmtl/MatrixOps.h"

namespace saint {
DefaultTimingEstimator::DefaultTimingEstimator(std::vector<float> onsetTimes)
    : _onsetTimes(std::move(onsetTimes)) {}

void DefaultTimingEstimator::addAttack(const std::chrono::milliseconds &time,
                                       size_t noteIndex) {
  while (_coordinates.size() >= fittingOrder) {
    _coordinates.erase(_coordinates.begin());
  }
  _coordinates.emplace_back(time, _onsetTimes[noteIndex]);
}

float DefaultTimingEstimator::estimateNoteIndex(
    const std::chrono::milliseconds &time) {
  gmtl::Matrix<float, fittingOrder, 2> A;
  gmtl::Matrix<float, fittingOrder, 1> B;
  for (auto i = 0u; i < _coordinates.size(); ++i) {
    const auto &coo = _coordinates[i];
    A.mData[i * 2u] = static_cast<float>(coo.first.count());
    A.mData[i * 2u + 1u] = 1.f;
    B.mData[i] = coo.second;
  }
  gmtl::Matrix<float, 2, fittingOrder> At;
  gmtl::transpose(At, A);
  gmtl::Matrix<float, 2, 2> inv;
  const auto prod = At * A;
  const auto x = gmtl::invert(inv, At * A) * At * B;
  return x[0][0] * static_cast<float>(time.count()) + x[1][0];
}
} // namespace saint