#include "DefaultPitchMapperHelper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

namespace saint {

constexpr Key A{PC::A, Mode::major};
constexpr Key Am{PC::A, Mode::minor};
constexpr Key Em{PC::E, Mode::minor};
constexpr Key Dm{PC::D, Mode::minor};

constexpr auto A4 = 69;

TEST(DefaultPitchMapperHelper, nnToAeolianDegree) {
  using Sut = DefaultPitchMapperHelper;

  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + 0, Am), 0);
  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + 2, Am), 1);
  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + 3, Am), 2);
  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + 12, Am), 7);
  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + -12, Am), -7);

  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + 7, Em), 0);
  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + 0, Em), -4);
  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + -5, Em), -7);

  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + 5, Dm), 0);
  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + 0, Dm), -3);
  EXPECT_FLOAT_EQ(Sut::nnToAeolianDegree(A4 + -7, Dm), -7);
}

TEST(DefaultPitchMapperHelper, aeolianDegreeToNn) {
  using Sut = DefaultPitchMapperHelper;

  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(0, Am), 0 + A4);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(1, Am), 2 + A4);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(2, Am), 3 + A4);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(7, Am), 12 + A4);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(-7, Am), -12 + A4);

  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(0, Em), 7 + A4);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(-4, Em), 0 + A4);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(-7, Em), -5 + A4);

  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(0, Dm), 5 + A4);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(-3, Dm), 0 + A4);
  EXPECT_FLOAT_EQ(Sut::aeolianDegreeToNn(-7, Dm), -7 + A4);
}

TEST(DefaultPitchMapperHelper, harmonized) {

  constexpr auto unison = 0;
  constexpr auto third = 2;
  constexpr auto fifth = 4;
  using Sut = DefaultPitchMapperHelper;
  EXPECT_FLOAT_EQ(Sut::harmonized(A4, A, unison), A4);
  EXPECT_FLOAT_EQ(Sut::harmonized(A4, Am, third), A4 + 3);
  EXPECT_FLOAT_EQ(Sut::harmonized(A4, A, third), A4 + 4);
  EXPECT_FLOAT_EQ(Sut::harmonized(A4, A, fifth), A4 + 7);
  EXPECT_FLOAT_EQ(Sut::harmonized(A4 + 12, A, fifth), A4 + 12 + 7);
  EXPECT_FLOAT_EQ(Sut::harmonized(A4 - 12, A, fifth), A4 - 12 + 7);
}
} // namespace saint
