#include "DefaultMelodyTracker.h"
#include "MelodyTracker/TimingEstimator/TimingEstimator.h"

#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace saint {
using namespace ::testing;
using namespace std::literals::chrono_literals;

class MockMelodyRecognizer : public MelodyRecognizer {
public:
  MOCK_METHOD1(
      onNoteOff,
      bool(const std::vector<std::pair<std::chrono::milliseconds, float>> &));
  MOCK_METHOD0(getNextNoteIndex, size_t());
};

class MockTimingEstimator : public TimingEstimator {
public:
  MOCK_METHOD2(addAttack,
               void(const std::chrono::milliseconds &, size_t noteIndex));
  MOCK_METHOD1(estimateNoteIndex, float(const std::chrono::milliseconds &));
};

TEST(DefaultMelodyTracker, stuff) {
  auto melodyRecognizer = new NiceMock<MockMelodyRecognizer>;
  auto timingEstimator = new NiceMock<MockTimingEstimator>;
  DefaultMelodyTracker sut{std::unique_ptr<MelodyRecognizer>{melodyRecognizer},
                           std::unique_ptr<TimingEstimator>{timingEstimator}};

  EXPECT_EQ(sut.onNoteOnSample(0ms, 60.f), 0);
  EXPECT_CALL(*melodyRecognizer, onNoteOff).WillOnce(Return(false));
  sut.onNoteOff();

  EXPECT_EQ(sut.onNoteOnSample(0ms, 60.f), 1);
  EXPECT_CALL(*melodyRecognizer, onNoteOff).WillOnce(Return(true));
  EXPECT_CALL(*melodyRecognizer, getNextNoteIndex).WillOnce(Return(2u));
  sut.onNoteOff();

  EXPECT_EQ(sut.onNoteOnSample(0ms, 60.f), 2u);
  // Can't recognize, entering "gliding mode".
  EXPECT_CALL(*melodyRecognizer, onNoteOff).WillOnce(Return(false));
  sut.onNoteOff();
  Mock::VerifyAndClearExpectations(melodyRecognizer);
  // Next time we call onNoteOff, will ask the TimingEstimator for the index
  EXPECT_CALL(*timingEstimator, estimateNoteIndex(100ms))
      .WillOnce(Return(123.f));
  EXPECT_EQ(sut.onNoteOnSample(0ms, 60.f), 123u);

  // This time it should be looking good again, just incrementing the index.
  EXPECT_CALL(*timingEstimator, addAttack);
  sut.onNoteOff();
  EXPECT_EQ(sut.onNoteOnSample(0ms, 60.f), 124u);
}
} // namespace saint
