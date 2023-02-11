#include "HarmoPitchFileReader.h"
#include "HarmoPitchGetter.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <optional>

using namespace saint;

namespace {
float getPitch(int noteNumber) {
  return 440 * std::powf(2, (noteNumber - 69) / 12.f);
}
} // namespace

TEST(HarmoPitchGetterTests, unit_test) {
  using OptPlayedNote = std::optional<PlayedNote>;
  constexpr OptPlayedNote noNote;
  constexpr OptPlayedNote aloneA4{{69}};
  constexpr OptPlayedNote minor3rdB4{{71, 3}};
  constexpr OptPlayedNote major3rdB4{{71, 4}};
  HarmoPitchGetter sut{{{0, noNote},
                        {3, aloneA4},
                        {5, minor3rdB4},
                        {8, major3rdB4},
                        {13, noNote}}};

  // Anywhere within [0, 5) should return no harmony
  EXPECT_EQ(std::nullopt, sut.getHarmoPitch(0, 440.f));
  EXPECT_EQ(std::nullopt, sut.getHarmoPitch(2, 440.f));
  EXPECT_EQ(std::nullopt, sut.getHarmoPitch(3, 440.f));
  EXPECT_EQ(std::nullopt, sut.getHarmoPitch(4, 440.f));

  auto interval = 3;
  EXPECT_FLOAT_EQ(*sut.getHarmoPitch(5, getPitch(71)), getPitch(71 + interval));
  // Anything else is just linear intra/extrapolation
  EXPECT_FLOAT_EQ(*sut.getHarmoPitch(5, getPitch(72)), getPitch(72 + interval));
  EXPECT_FLOAT_EQ(*sut.getHarmoPitch(5, getPitch(70)), getPitch(70 + interval));
  EXPECT_FLOAT_EQ(*sut.getHarmoPitch(5, getPitch(100)),
                  getPitch(100 + interval));

  // Now harmonizing by a major 3rd
  interval = 4;
  EXPECT_FLOAT_EQ(*sut.getHarmoPitch(8, getPitch(71)), getPitch(71 + interval));
  EXPECT_FLOAT_EQ(*sut.getHarmoPitch(8, getPitch(72)), getPitch(72 + interval));
  EXPECT_FLOAT_EQ(*sut.getHarmoPitch(8, getPitch(70)), getPitch(70 + interval));
  EXPECT_FLOAT_EQ(*sut.getHarmoPitch(8, getPitch(100)),
                  getPitch(100 + interval));
}

TEST(HarmoPitchGetterTests, with_real_midi_input) {
  const auto input =
      toHarmoPitchGetterInput("./saint/_assets/Hotel_California.xml");
  HarmoPitchGetter sut{input};
  const auto result = sut.getHarmoPitch(0, 0.f);
}
