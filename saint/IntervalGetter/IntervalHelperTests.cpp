#include "IntervalHelper.h"

#include <gtest/gtest.h>
#include <optional>

namespace saint {

TEST(IntervalHelper, setIntervalIndex) {
  const std::vector<int> intervals{{1, 2, 3, 5, 8}};
  auto currentIndex = 0;
  EXPECT_FALSE(setIntervalIndex(intervals, currentIndex, 0 /*tick*/));

  EXPECT_TRUE(setIntervalIndex(intervals, currentIndex, 1));
  EXPECT_EQ(0, currentIndex);

  EXPECT_TRUE(setIntervalIndex(intervals, currentIndex, 2));
  EXPECT_EQ(1, currentIndex);

  EXPECT_TRUE(setIntervalIndex(intervals, currentIndex, 4));
  EXPECT_EQ(2, currentIndex);

  // Last interval is closed
  EXPECT_TRUE(setIntervalIndex(intervals, currentIndex, 8));
  EXPECT_EQ(4, currentIndex);

  EXPECT_FALSE(setIntervalIndex(intervals, currentIndex, 9));
  EXPECT_EQ(4, currentIndex); // Value of current index isn't modified

  EXPECT_TRUE(setIntervalIndex(intervals, currentIndex, 2));
  EXPECT_EQ(1, currentIndex);

  EXPECT_TRUE(setIntervalIndex(intervals, currentIndex, 1));
  EXPECT_EQ(0, currentIndex);

  EXPECT_FALSE(setIntervalIndex(intervals, currentIndex, 0));
  EXPECT_EQ(0, currentIndex);
}

TEST(IntervalHelper, toIntervalSpans) {
  const std::vector<MidiNoteMsg> playedMidiTrack{{
                                                     1,    // tick
                                                     true, // isNoteOn
                                                     69,   // noteNumber
                                                 },
                                                 {2, false, 69},
                                                 {2, true, 71},
                                                 {10, false, 71}};
  const std::vector<MidiNoteMsg> harmoMidiTrack{{2, true, 74}, {10, false, 74}};
  const std::vector<IntervalSpan> actual =
      toIntervalSpans(playedMidiTrack, harmoMidiTrack);

  const std::vector<IntervalSpan> expected{{0, std::nullopt},
                                           {1, PlayedNote{69, std::nullopt}},
                                           {2, PlayedNote{71, 3}},
                                           {10, std::nullopt}};

  EXPECT_EQ(expected, actual);
}

} // namespace saint
