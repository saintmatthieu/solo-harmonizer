#include <numeric>
#define _USE_MATH_DEFINES

#include <array>
#include <gtest/gtest.h>

#include "SbsmsWrapper.h"
#include <algorithm>
#include <math.h>

namespace {
std::vector<float> getSinewave(size_t T, size_t N) {
  std::vector<float> sinewave(N);
  const auto dT = (double)T;
  const auto dN = (double)N;
  for (auto i = 0u; i < N; ++i) {
    sinewave[i] = std::cosf((float)(i * 2 * M_PI * dT / dN));
  }
  return sinewave;
}

template <typename T>
std::vector<T> getDiff(const std::vector<T> &A, const std::vector<T> &B) {
  std::vector<T> diff(A.size());
  for (auto i = 0u; i < A.size(); ++i) {
    diff[i] = B[i] - A[i];
  }
  return diff;
}

float getRms(const std::vector<float> &V) {
  auto U = V;
  std::transform(V.begin(), V.end(), U.begin(), [](auto v) { return v * v; });
  return std::sqrtf(std::accumulate(U.begin(), U.end(), 0.f) / (float)V.size());
}
} // namespace

TEST(SbsmsWrapperTest, DoesStuff) {
  SbsmsWrapper sut;
  const auto blockSize = sut.getInputFrameSize();
  const auto numBlocks = 2;
  const auto N = blockSize * numBlocks;
  const auto reference = getSinewave(numBlocks, N);
  auto processed = reference;
  for (auto i = 0u; i < numBlocks; ++i) {
    sut.process(processed.data() + i * blockSize, (long)blockSize);
  }
  const auto diffRms = getRms(getDiff(reference, processed));
  const auto minus90dB = std::powf(10.f, -90 / 20.f);
  EXPECT_NEAR(0.f, diffRms, minus90dB);
}