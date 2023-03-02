#include "IntervalGetter.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <optional>

using namespace saint;

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

TEST(IntervalGetter, returns_nullopt_when_tick_is_outside_boundaries) {
  IntervalGetter sut{{
                         {1, minor3rdB4},
                         {2, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_EQ(std::nullopt, sut.getHarmoInterval(0, 440.f));
  EXPECT_EQ(3, sut.getHarmoInterval(1, 440.f));
  EXPECT_EQ(std::nullopt, sut.getHarmoInterval(2, 440.f));
}

TEST(IntervalGetter, returns_nullopt_if_no_interval) {
  IntervalGetter sut{{
                         {0, aloneA4}, // not an interval, just a note by itself
                         {1, noNote},
                     },
                     ticksPerCrotchet};
  EXPECT_EQ(std::nullopt, sut.getHarmoInterval(0, 440.f));
}

TEST(IntervalGetter,
     currently_returns_constant_interval_independently_on_input_pitch) {
  IntervalGetter sut{{
                         {0, minor3rdB4},
                         {1, noNote},
                     },
                     ticksPerCrotchet};
  const auto expectedInterval = 3;
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(0, getPitch(0)), expectedInterval);
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(0, getPitch(100)), expectedInterval);
}
