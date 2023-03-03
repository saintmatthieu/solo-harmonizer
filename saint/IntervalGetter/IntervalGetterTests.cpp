#include "IntervalGetter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <optional>

using namespace saint;
using namespace ::testing;

namespace {
float getPitch(int noteNumber) {
  return 440 * std::powf(2, (noteNumber - 69) / 12.f);
}
constexpr auto ticksPerCrotchet = 1.0;
using OptPlayedNote = std::optional<PlayedNote>;
constexpr OptPlayedNote noNote;
constexpr OptPlayedNote aloneA4{{69}};
constexpr OptPlayedNote minor3rdB4{{71, 3}};
constexpr OptPlayedNote major3rdB4{{71, 4}};
} // namespace

TEST(IntervalGetter, returns_nullopt_if_no_interval) {
  IntervalGetter sut{{
                         {0, aloneA4}, // not an interval, just a note by itself
                         {1, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_THAT(sut.getHarmoInterval(0, 123.f), Eq(std::nullopt));
}

TEST(IntervalGetter,
     currently_returns_constant_interval_independently_on_input_pitch) {
  IntervalGetter sut{{
                         {0, minor3rdB4},
                         {1, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_THAT(sut.getHarmoInterval(0, 0.f), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(0, 100.f), Optional(3.f));
}

// We are only allowed to have interval jump when there is a discontinuity in
// the incoming pitch. A discontinuity is any pitch transition other than
// pitch.has_value() -> pitch.has_value().

TEST(IntervalGetter, natural_scenario) {
  // In this test we try a "natural" scenario.
  // In following tests we'll try the transitions one by one.

  IntervalGetter sut{{
                         {0, minor3rdB4},
                         {2, major3rdB4},
                         {5, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_THAT(sut.getHarmoInterval(0, 123.f), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(1, 234.f), Optional(3.f));
  // This tick cuts a major 3rd, but the interval state sticks because the pitch
  // state is a yes-yes transition.
  EXPECT_THAT(sut.getHarmoInterval(2, 345.f), Optional(3.f));
  // yes-no transition -> we switch asap to the next interval, to keep delay as
  // low as possible.
  EXPECT_THAT(sut.getHarmoInterval(3, std::nullopt), Optional(4.f));
  EXPECT_THAT(sut.getHarmoInterval(4, 456.f), Optional(4.f));
  // yes-yes transition -> the interval is preserved. That the tick doesn't cut
  // any interval doesn't matter.
  EXPECT_THAT(sut.getHarmoInterval(5, 456.f), Optional(4.f));
  // yes-no transition -> we let go.
  EXPECT_THAT(sut.getHarmoInterval(6, std::nullopt), Eq(std::nullopt));
}

TEST(IntervalGetter, no_pitch_to_no_pitch) {
  IntervalGetter sut{{
                         {0, noNote},
                         {1, minor3rdB4},
                         {2, major3rdB4},
                         {3, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_THAT(sut.getHarmoInterval(0, std::nullopt), Eq(std::nullopt));
  EXPECT_THAT(sut.getHarmoInterval(1, std::nullopt), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(2, std::nullopt), Optional(4.f));
  EXPECT_THAT(sut.getHarmoInterval(3, std::nullopt), Eq(std::nullopt));
}

TEST(IntervalGetter, no_pitch_to_yes_pitch) {
  IntervalGetter sut{{
                         {0, minor3rdB4},
                         {1, major3rdB4},
                         {2, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_THAT(sut.getHarmoInterval(0, std::nullopt), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(1, 123.f), Optional(4.f));
}

TEST(IntervalGetter, yes_pitch_to_yes_pitch) {
  IntervalGetter sut{{
                         {0, minor3rdB4},
                         {1, major3rdB4},
                         {2, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_THAT(sut.getHarmoInterval(0, 123.f), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(1, 123.f), Optional(3.f));
}

TEST(IntervalGetter, yes_pitch_to_no_pitch) {
  IntervalGetter sut{{
                         {0, minor3rdB4},
                         {1, major3rdB4},
                         {2, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_THAT(sut.getHarmoInterval(0, 123.f), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(1, std::nullopt), Optional(4.f));
}
