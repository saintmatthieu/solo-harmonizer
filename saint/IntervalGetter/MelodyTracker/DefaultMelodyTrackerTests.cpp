#include "DefaultMelodyTracker.h"
#include "MelodyTracker/TimingEstimator/TimingEstimator.h"

#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace saint {
using namespace ::testing;
using namespace std::literals::chrono_literals;

namespace {
constexpr auto A = 57;
constexpr auto B = 59;
constexpr auto C = 60;
constexpr auto D = 62;
constexpr auto E = 64;
constexpr auto F = 65;
const std::vector<std::pair<float, std::optional<int>>> timedMelody{
    {0, {A}},  {1, {B}},  {2, {C}},  {3, {D}},  {4, {E}},  {5, {F}},
    {6, {E}},  {7, {D}},  {8, {D}},  {9, {C}},  {10, {C}}, {11, {B}},
    {12, {C}}, {13, {D}}, {14, {E}}, {15, {C}}, {16, {B}}, {17, {A}}};
} // namespace

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

class MockClock : public Clock {
public:
  MOCK_CONST_METHOD0(now, std::chrono::milliseconds());
};

TEST(DefaultMelodyTracker, stuff) {
  auto melodyRecognizer = new NiceMock<MockMelodyRecognizer>;
  auto timingEstimator = new NiceMock<MockTimingEstimator>;
  auto clock = new NiceMock<MockClock>;
  DefaultMelodyTracker sut{timedMelody,
                           std::unique_ptr<MelodyRecognizer>{melodyRecognizer},
                           std::unique_ptr<TimingEstimator>{timingEstimator},
                           std::unique_ptr<Clock>{clock}};

  ON_CALL(*clock, now).WillByDefault(Return(0ms));

  EXPECT_EQ(sut.onNoteOnSample(60.f), 0);
  EXPECT_CALL(*melodyRecognizer, onNoteOff).WillOnce(Return(false));
  sut.onNoteOff();

  EXPECT_EQ(sut.onNoteOnSample(60.f), 1);
  EXPECT_CALL(*melodyRecognizer, onNoteOff).WillOnce(Return(true));
  EXPECT_CALL(*melodyRecognizer, getNextNoteIndex).WillOnce(Return(2u));
  sut.onNoteOff();

  EXPECT_EQ(sut.onNoteOnSample(60.f), 2u);
  // Can't recognize, entering "gliding mode".
  EXPECT_CALL(*melodyRecognizer, onNoteOff).WillOnce(Return(false));
  sut.onNoteOff();
  Mock::VerifyAndClearExpectations(melodyRecognizer);
  // Next time we call onNoteOff, will ask the TimingEstimator for the index
  EXPECT_CALL(*clock, now).WillOnce(Return(100ms));
  EXPECT_CALL(*timingEstimator, estimateNoteIndex(100ms))
      .WillOnce(Return(123.f));
  EXPECT_EQ(sut.onNoteOnSample(60.f), 123u);
  Mock::VerifyAndClearExpectations(clock);

  // This time it should be looking good again, just incrementing the index.
  EXPECT_CALL(*timingEstimator, addAttack);
  sut.onNoteOff();
  EXPECT_EQ(sut.onNoteOnSample(60.f), 124u);
}
} // namespace saint
