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
  constexpr OptPlayedNote harmonizedB4{{71, 3}};
  constexpr OptPlayedNote harmonizedC4{{72, 4}};
  HarmoPitchGetter sut{{{1, noNote},
                        {2, aloneA4},
                        {3, harmonizedB4},
                        {5, harmonizedC4},
                        {8, noNote}}};

  // Anywhere within [0, 3) should return no harmony
  EXPECT_EQ(sut.intervalInSemitonesAtTick(0), std::nullopt);
  EXPECT_EQ(sut.intervalInSemitonesAtTick(2), std::nullopt);

  {
    const auto result = sut.intervalInSemitonesAtTick(4);
    ASSERT_NE(result, std::nullopt);
    EXPECT_FLOAT_EQ(*result, 3.5f);
  }
  {
    const auto result = sut.intervalInSemitonesAtTick(6);
    ASSERT_NE(result, std::nullopt);
    EXPECT_FLOAT_EQ(*result, 4.f);
  }
}

TEST(HarmoPitchGetterTests, with_real_midi_input) {
  const auto input =
      toHarmoPitchGetterInput("./saint/_assets/Hotel_California.xml");
  HarmoPitchGetter sut{input};
  const auto result = sut.intervalInSemitonesAtTick(0);
}
