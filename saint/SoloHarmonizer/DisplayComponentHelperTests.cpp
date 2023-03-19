#include "DisplayComponentHelper.h"

#include <gtest/gtest.h>

namespace saint {
TEST(setToImageWidth, emptyInput_doesntCrash) {
  const std::vector<IntervalSpan> spans;
  auto first = spans.begin();
  auto last = spans.begin();
  setToImageWidth(spans, first, last, 0.f);
}

TEST(setToImageWidth, stuff) {
  const std::vector<IntervalSpan> spans{{10.f, {{69, std::nullopt}}},
                                        {10.f, {{69, std::nullopt}}}};
}
} // namespace saint
