#include "KrumhanslSchmucklerAlgorithm.h"

#include <gtest/gtest.h>

namespace saint {
TEST(KrumhanslSchmucklerAlgorithm, works) {
  using KSA = KrumhanslSchmucklerAlgorithm;
  const std::vector<int> cTriad{60, 64, 67};
  const std::vector<int> cSharpTriad{61, 65, 68};
  const std::vector<int> aMinorEMajor{57, 60, 64, 64, 68, 71};
  constexpr Key C{PC::C, Mode::major};
  constexpr Key Csh{PC::Csh, Mode::major};
  constexpr Key Am{PC::A, Mode::minor};
  EXPECT_EQ(KSA::getMostLikelyKey(cTriad), C);
  EXPECT_EQ(KSA::getMostLikelyKey(cSharpTriad), Csh);
  EXPECT_EQ(KSA::getMostLikelyKey(aMinorEMajor), Am);
}
} // namespace saint
