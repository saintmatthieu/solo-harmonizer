#include "JuceMidiFileUtils.h"
#include "PositionGetter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace saint {

TEST(PositionGetter, stuff) {
  const auto timeSignatures = getTimeSignatures(
      "./saint/_assets/fourFourThenSixEightThenThreeFourThenFourFour.mid");
  ASSERT_TRUE(timeSignatures.has_value());
  PositionGetter sut{*timeSignatures};
  const auto pos1 = sut.getPosition(0.f);
  const auto pos2 = sut.getPosition(4.f);
  const auto pos3 = sut.getPosition(8.f);
}

} // namespace saint
