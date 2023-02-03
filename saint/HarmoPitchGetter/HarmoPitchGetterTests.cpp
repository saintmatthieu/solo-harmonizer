#include "HarmoPitchGetter.h"
#include "HarmoPitchHelper.h"
#include <gtest/gtest.h>
#include <memory>
#include <optional>

using namespace saint;

namespace {
int toNoteNumber(PitchClass pitchClass) {
  switch (pitchClass) {
  case PitchClass::A:
    return 0;
  case PitchClass::B:
    return 2;
  case PitchClass::C:
    return 3;
  case PitchClass::D:
    return 5;
  case PitchClass::E:
    return 7;
  case PitchClass::F:
    return 8;
  case PitchClass::G:
    return 10;
  }
}

int toNoteNumber(const Note &note, int octave) {
  return toNoteNumber(note.pitchClass) + octave * 12 + (note.isSharp ? 1 : 0);
}

float getPitch(const Note &note, int octave = 0) {
  return 440 * std::powf(2, toNoteNumber(note, octave) / 12.f);
}
} // namespace

TEST(HarmoPitchGetterTests, unit_test) {
  constexpr auto isSharp = true;
  constexpr std::optional<ReferenceNote> noNote;
  constexpr std::optional<ReferenceNote> aloneA4{{Note{PitchClass::A}}};
  constexpr std::optional<ReferenceNote> harmonizedB4{
      {Note{PitchClass::B}, Note{PitchClass::D}}};
  HarmoPitchGetter sut{{{0, noNote}, {1, aloneA4}, {3, harmonizedB4}}};

  // Anywhere within [0, 3) should return no harmony
  EXPECT_EQ(sut.getHarmoPitch(0, 0.f), std::nullopt);
  EXPECT_EQ(sut.getHarmoPitch(1, 0.f), std::nullopt);

  const auto result = sut.getHarmoPitch(1, 0.f);
  ASSERT_NE(result, std::nullopt);
  EXPECT_FLOAT_EQ(*result, getPitch(Note{PitchClass::C}));
}

TEST(HarmoPitchGetterTests, with_real_midi_input) {
  const auto input =
      toHarmoPitchGetterInput("C:/Users/saint/Downloads/test.xml");
  HarmoPitchGetter sut{input};
  const auto result = sut.getHarmoPitch(0, 0.f);
}
