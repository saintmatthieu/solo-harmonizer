#include "JuceMidiFileUtils.h"
#include "PositionGetter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace saint {

using namespace ::testing;

TEST(PositionGetter, stuff) {
  const auto timeSignatures = getTimeSignatures(
      std::filesystem::absolute(
          "./saint/_assets/fourFourThenSixEightThenThreeFourThenFourFour.mid")
          .string());
  ASSERT_TRUE(timeSignatures.has_value());
  PositionGetter sut{*timeSignatures};
  EXPECT_THAT(sut.getPosition(0.f), Eq(Position({0, 0.f})));
  EXPECT_THAT(sut.getPosition(4.f), Eq(Position({1, 0.f})));
  EXPECT_THAT(sut.getPosition(8.f), Eq(Position({2, 1.f})));
  EXPECT_THAT(sut.getPosition(11.f), Eq(Position({3, 1.f})));
  EXPECT_THAT(sut.getPosition(15.f), Eq(Position({4, 1.f})));
}

} // namespace saint
