#include "DefaultPitchMapperHelper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

namespace saint {
TEST(DefaultPitchMapperHelper, semiToAeolianDegree) {
  using Sut = DefaultPitchMapperHelper;

  constexpr Key Am{PC::A, Mode::minor};
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(0, Am), 0);
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(2, Am), 1);
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(3, Am), 2);
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(12, Am), 7);
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(-12, Am), -7);

  constexpr Key Em{PC::E, Mode::minor};
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(7, Em), 0);
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(0, Em), -4);
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(-5, Em), -7);

  constexpr Key Dm{PC::D, Mode::minor};
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(5, Dm), 0);
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(0, Dm), -3);
  EXPECT_FLOAT_EQ(Sut::semiToAeolianDegree(-7, Dm), -7);
}

TEST(DefaultPitchMapperHelper, aeolianDegreeToSemi) {
  using Sut = DefaultPitchMapperHelper;

  constexpr Key Am{PC::A, Mode::minor};
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(0, Am), 0);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(1, Am), 2);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(2, Am), 3);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(7, Am), 12);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(-7, Am), -12);

  constexpr Key Em{PC::E, Mode::minor};
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(0, Em), 7);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(-4, Em), 0);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(-7, Em), -5);

  constexpr Key Dm{PC::D, Mode::minor};
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(0, Dm), 5);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(-3, Dm), 0);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToSemi(-7, Dm), -7);
}

TEST(DefaultPitchMapperHelper, harmonized) {
  constexpr Key A{PC::A, Mode::major};
  constexpr Key Am{PC::A, Mode::minor};
  constexpr auto unison = 0;
  constexpr auto third = 2;
  constexpr auto fifth = 4;
  using Sut = DefaultPitchMapperHelper;
  EXPECT_FLOAT_EQ(Sut::harmonized(0.f, A, unison), 0.f);
  EXPECT_FLOAT_EQ(Sut::harmonized(0.f, Am, third), 3.f);
  EXPECT_FLOAT_EQ(Sut::harmonized(0.f, A, third), 4.f);
  EXPECT_FLOAT_EQ(Sut::harmonized(0.f, A, fifth), 7.f);
  EXPECT_FLOAT_EQ(Sut::harmonized(12.f, A, fifth), 12.f + 7.f);
  EXPECT_FLOAT_EQ(Sut::harmonized(-12.f, A, fifth), -12.f + 7.f);
}
} // namespace saint
