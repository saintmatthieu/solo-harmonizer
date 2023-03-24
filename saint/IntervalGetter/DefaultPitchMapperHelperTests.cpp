#include "DefaultPitchMapperHelper.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

namespace saint {
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
