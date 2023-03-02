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
} // namespace

TEST(IntervalGetterTests, unit_test) {
  using OptPlayedNote = std::optional<PlayedNote>;
  constexpr OptPlayedNote noNote;
  constexpr OptPlayedNote aloneA4{{69}};
  constexpr OptPlayedNote minor3rdB4{{71, 3}};
  constexpr OptPlayedNote major3rdB4{{71, 4}};
  IntervalGetter sut{{{0, noNote},
                      {3, aloneA4},
                      {5, minor3rdB4},
                      {8, major3rdB4},
                      {13, noNote}},
                     ticksPerCrotchet};

  // Anywhere within [0, 5) should return no harmony
  EXPECT_EQ(std::nullopt, sut.getHarmoInterval(0, 440.f));
  EXPECT_EQ(std::nullopt, sut.getHarmoInterval(2, 440.f));
  EXPECT_EQ(std::nullopt, sut.getHarmoInterval(3, 440.f));
  EXPECT_EQ(std::nullopt, sut.getHarmoInterval(4, 440.f));

  auto expectedInterval = 3;
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(5, getPitch(71)), expectedInterval);
  // Anything else is just linear intra/extrapolation
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(5, getPitch(72)), expectedInterval);
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(5, getPitch(70)), expectedInterval);
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(5, getPitch(100)), expectedInterval);

  // Now harmonizing by a major 3rd
  expectedInterval = 4;
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(8, getPitch(71)), expectedInterval);
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(8, getPitch(72)), expectedInterval);
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(8, getPitch(70)), expectedInterval);
  EXPECT_FLOAT_EQ(*sut.getHarmoInterval(8, getPitch(100)), expectedInterval);
}
