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
  constexpr std::optional<PlayedNote> noNote;
  constexpr std::optional<PlayedNote> aloneA4{{69}};
  constexpr std::optional<PlayedNote> harmonizedB4{{71, 74}};
  HarmoPitchGetter sut{{{0, noNote}, {1, aloneA4}, {3, harmonizedB4}}};

  // Anywhere within [0, 3) should return no harmony
  EXPECT_EQ(sut.getHarmoPitch(0, 0.f), std::nullopt);
  EXPECT_EQ(sut.getHarmoPitch(1, 0.f), std::nullopt);

  const auto result = sut.getHarmoPitch(1, 0.f);
  ASSERT_NE(result, std::nullopt);
  EXPECT_FLOAT_EQ(*result, getPitch(72));
}

TEST(HarmoPitchGetterTests, with_real_midi_input) {
  const auto input =
      toHarmoPitchGetterInput("./saint/_assets/Hotel_California.xml");
  HarmoPitchGetter sut{input};
  const auto result = sut.getHarmoPitch(0, 0.f);
}
