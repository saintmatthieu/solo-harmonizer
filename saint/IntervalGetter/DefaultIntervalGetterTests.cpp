#include "DefaultIntervalGetter.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <optional>

using namespace saint;
using namespace ::testing;

namespace {
constexpr auto ticksPerSample = 1.f;
using OptPlayedNote = std::optional<PlayedNote>;
constexpr OptPlayedNote noNote;
constexpr OptPlayedNote aloneA4{{69}};
constexpr OptPlayedNote minor3rdB4{{71, 3}};
constexpr OptPlayedNote major3rdB4{{71, 4}};
} // namespace

TEST(DefaultIntervalGetter, returns_nullopt_if_no_interval) {
  DefaultIntervalGetter sut{
      {
          {0.f, aloneA4}, // not an interval, just a note by itself
          {1.f, noNote},
      },
      std::nullopt};
  EXPECT_THAT(sut.getHarmoInterval(0.f, 123.f), Eq(std::nullopt));
}

TEST(DefaultIntervalGetter,
     currently_returns_constant_interval_independently_on_input_pitch) {
  DefaultIntervalGetter sut{{
                                {0.f, minor3rdB4},
                                {1.f, noNote},
                            },
                            std::nullopt};
  EXPECT_THAT(sut.getHarmoInterval(0.f, 0.f), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(0.f, 100.f), Optional(3.f));
}

// We are only allowed to have interval jump when there is a discontinuity in
// the incoming pitch. A discontinuity is any pitch transition other than
// pitch.has_value() -> pitch.has_value().

TEST(DefaultIntervalGetter, natural_scenario) {
  // In this test we try a "natural" scenario.
  // In following tests we'll try the transitions one by one.

  DefaultIntervalGetter sut{{
                                {0.f, minor3rdB4},
                                {2.f, major3rdB4},
                                {5.f, noNote},
                            },
                            std::nullopt};
  EXPECT_THAT(sut.getHarmoInterval(0.f, 123.f), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(1.f, 234.f), Optional(3.f));
  // This tick cuts a major 3rd, but the interval state sticks because the pitch
  // state is a yes-yes transition.
  EXPECT_THAT(sut.getHarmoInterval(2.f, 345.f), Optional(3.f));
  // yes-no transition -> jumps to closest
  EXPECT_THAT(sut.getHarmoInterval(3.f, std::nullopt), Optional(4.f));
  EXPECT_THAT(sut.getHarmoInterval(3.f, 456.f), Optional(4.f));
  EXPECT_THAT(sut.getHarmoInterval(4.f, 456.f), Optional(4.f));
  // yes-yes transition -> the interval is preserved. That the tick doesn't cut
  // any interval doesn't matter.
  EXPECT_THAT(sut.getHarmoInterval(5.f, 456.f), Optional(4.f));
  // yes-no transition -> we let go.
  EXPECT_THAT(sut.getHarmoInterval(6.f, std::nullopt), Eq(std::nullopt));
}

TEST(DefaultIntervalGetter, no_pitch_to_no_pitch) {
  DefaultIntervalGetter sut{{
                                {0.f, noNote},
                                {1.f, minor3rdB4},
                                {2.f, major3rdB4},
                                {3.f, noNote},
                            },
                            std::nullopt};
  EXPECT_THAT(sut.getHarmoInterval(0.f, std::nullopt), Eq(std::nullopt));
  EXPECT_THAT(sut.getHarmoInterval(1.f, std::nullopt), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(2.f, std::nullopt), Optional(4.f));
  EXPECT_THAT(sut.getHarmoInterval(3.f, std::nullopt), Eq(std::nullopt));
}

TEST(DefaultIntervalGetter, no_pitch_to_yes_pitch) {
  DefaultIntervalGetter sut{{
                                {0.f, minor3rdB4},
                                {1.f, major3rdB4},
                                {2.f, noNote},
                            },
                            std::nullopt};
  EXPECT_THAT(sut.getHarmoInterval(0.f, std::nullopt), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(1.f, 123.f), Optional(4.f));
}

TEST(DefaultIntervalGetter, yes_pitch_to_yes_pitch) {
  DefaultIntervalGetter sut{{
                                {0.f, minor3rdB4},
                                {1.f, major3rdB4},
                                {2.f, noNote},
                            },
                            std::nullopt};
  EXPECT_THAT(sut.getHarmoInterval(0.f, 123.f), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(1.f, 123.f), Optional(3.f));
}

TEST(DefaultIntervalGetter, yes_pitch_to_no_pitch) {
  DefaultIntervalGetter sut{{
                                {0.f, minor3rdB4},
                                {1.f, major3rdB4},
                                {2.f, noNote},
                            },
                            std::nullopt};
  EXPECT_THAT(sut.getHarmoInterval(0.f, 123.f), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(1.f, std::nullopt), Optional(4.f));
}

TEST(DefaultIntervalGetter, snaps_interval_to_onset_closest_to_playhead) {
  DefaultIntervalGetter sut{{
                                {0.f, aloneA4},
                                {3.f, minor3rdB4},
                                {6.f, major3rdB4},
                                {9.f, noNote},
                            },
                            std::nullopt};
  EXPECT_THAT(sut.getHarmoInterval(1.f, std::nullopt), Eq(std::nullopt));
  EXPECT_THAT(sut.getHarmoInterval(4.f, std::nullopt), Optional(3.f));
  EXPECT_THAT(sut.getHarmoInterval(5.f, std::nullopt), Optional(4.f));
  EXPECT_THAT(sut.getHarmoInterval(7.f, std::nullopt), Optional(4.f));
  EXPECT_THAT(sut.getHarmoInterval(8.f, std::nullopt), Optional(4.f));
}
