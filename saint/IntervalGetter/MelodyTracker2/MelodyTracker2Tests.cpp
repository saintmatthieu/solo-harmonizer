#include "MelodyTracker2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace saint {
using namespace ::testing;
TEST(MelodyTracker2, Stuff) {
  const MelodyTracker2::Melody melody{
      {1.f, 60},
      {2.f, 62},
      {1.f, 60},
  };
  MelodyTracker2 sut(melody);
  EXPECT_EQ(sut.onNoteOff({61.f, 59.f}), std::nullopt);
  EXPECT_THAT(sut.onNoteOff({63.f, 61.f, 62.f}), Optional(1u));
  EXPECT_THAT(sut.onNoteOff({61.f, 59.f}), Optional(2u));
}
} // namespace saint
