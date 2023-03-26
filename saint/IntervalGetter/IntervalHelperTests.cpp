#include "IntervalHelper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <optional>

namespace saint {

using namespace ::testing;

TEST(getClosestLimitIndex, leading_round_up_rule) {
  // We'll want the playhead to snap to the first interval slightly before it
  // has reached it already. What does "slightly before" mean ? We say it's half
  // the duration of the first interval.
  const std::vector<float> intervalExample1{
      {3.f, 7.f}}; // Should snap to first interval from 1 already.
  const std::vector<float> intervalExample2{
      {3.f, 5.f}}; // Should snapt to first interval from 2.

  EXPECT_THAT(getClosestLimitIndex(intervalExample1, 1.f), Optional(0));
  EXPECT_THAT(getClosestLimitIndex(intervalExample2, 1.f), Eq(std::nullopt));
  EXPECT_THAT(getClosestLimitIndex(intervalExample2, 2.f), Optional(0));
}

TEST(getClosestLimitIndex, various_tests) {
  const std::vector<float> intervals{{2.f, 3.f, 5.f, 8.f}};
  EXPECT_THAT(getClosestLimitIndex(intervals, 0.f), Eq(std::nullopt));
  EXPECT_THAT(getClosestLimitIndex(intervals, 1.f), Eq(std::nullopt));
  EXPECT_THAT(getClosestLimitIndex(intervals, 2.f), Optional(0));
  EXPECT_THAT(getClosestLimitIndex(intervals, 3.f), Optional(1));
  // 4 is exactly in the middle of [3, 5) -> rounds up.
  EXPECT_THAT(getClosestLimitIndex(intervals, 4.f), Optional(2));
  EXPECT_THAT(getClosestLimitIndex(intervals, 5.f), Optional(2));
  EXPECT_THAT(getClosestLimitIndex(intervals, 6.f), Optional(2));
  EXPECT_THAT(getClosestLimitIndex(intervals, 7.f), Eq(std::nullopt));
  EXPECT_THAT(getClosestLimitIndex(intervals, 8.f), Eq(std::nullopt));
  EXPECT_THAT(getClosestLimitIndex(intervals, 9.f), Eq(std::nullopt));
}

TEST(toIntervalSpans, variousTests) {
  const std::vector<MidiNoteMsg> playedMidiTrack{{
                                                     1.f,  // crotchet
                                                     true, // isNoteOn
                                                     69,   // noteNumber
                                                 },
                                                 {2.f, false, 69},
                                                 {2.f, true, 71},
                                                 {10.f, false, 71}};
  const std::vector<MidiNoteMsg> harmoMidiTrack{{2.f, true, 74},
                                                {10.f, false, 74}};
  const std::vector<IntervalSpan> actual =
      toIntervalSpans(playedMidiTrack, harmoMidiTrack, {});

  const std::vector<IntervalSpan> expected{{0.f, std::nullopt},
                                           {1.f, PlayedNote{69, std::nullopt}},
                                           {2.f, PlayedNote{71, 3}},
                                           {10.f, std::nullopt}};

  EXPECT_EQ(expected, actual);
}

} // namespace saint
