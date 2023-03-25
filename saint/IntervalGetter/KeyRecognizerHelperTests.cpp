#include "KeyRecognizerHelper.h"

#include <algorithm>
#include <gtest/gtest.h>

namespace saint {

TEST(KeyRecognizerHelper, stuff) {
  const std::vector<float> crotchets{0.f, 1.f, 2.f, 13.f};
  const std::vector<SigPos> sigs{{0, {4, 4}}, {3, {3, 4}}};
  const std::map<size_t, size_t> expected{{0, 3}, {3, 4}};
  const auto actual = KeyRecognizerHelper::groupIndicesByBar(crotchets, sigs);
  EXPECT_EQ(expected, actual);
}

} // namespace saint
