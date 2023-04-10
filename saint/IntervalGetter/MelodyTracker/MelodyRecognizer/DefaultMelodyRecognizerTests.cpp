#include "DefaultMelodyRecognizer.h"

#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace saint {

using namespace ::testing;
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

const std::unordered_set<int> intervalSet{-1, -2, 1, 2};
} // namespace

class ObservationLikelihoodGetterFake : public ObservationLikelihoodGetter {
public:
  ObservationLikelihoodGetterFake(std::unordered_set<int> set)
      : hiddenStateSet(std::move(set)) {
    // At first we don't know shit, so return just equal likelihood for all
    // states
    for (auto state : hiddenStateSet) {
      observationLikelihoods[state] =
          std::logf(1.f / static_cast<float>(hiddenStateSet.size()));
    }
  }
  const std::unordered_set<int> hiddenStateSet;
  std::unordered_map<int, float> getObservationLogLikelihoods(
      const std::vector<std::pair<std::chrono::milliseconds, float>> &)
      override {
    return observationLikelihoods;
  }
  void setOnly(int nn, float prob) {
    for (auto state : hiddenStateSet) {
      observationLikelihoods[state] = state == nn ? 0.f : std::logf(0.f);
    }
  }
  std::unordered_map<int, float> observationLikelihoods;
};

TEST(DefaultMelodyRecognizer, easy) {
  auto likelihoodGetter = new ObservationLikelihoodGetterFake(intervalSet);
  DefaultMelodyRecognizer sut{
      std::unique_ptr<ObservationLikelihoodGetter>{likelihoodGetter},
      timedMelody};
  EXPECT_EQ(sut.getNextNoteIndex(), 0);
  EXPECT_EQ(sut.getNextNoteIndex(), 1);
  likelihoodGetter->setOnly(-1, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 2);
  likelihoodGetter->setOnly(-2, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 3);
  likelihoodGetter->setOnly(2, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 4);
  likelihoodGetter->setOnly(-2, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 5);
  likelihoodGetter->setOnly(2, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 6);
  likelihoodGetter->setOnly(1, 1.f);
}

TEST(DefaultMelodyRecognizer, beginFromTheMiddle) {
  auto likelihoodGetter = new ObservationLikelihoodGetterFake(intervalSet);
  DefaultMelodyRecognizer sut{
      std::unique_ptr<ObservationLikelihoodGetter>{likelihoodGetter},
      timedMelody};
  likelihoodGetter->setOnly(57, 1.f);
  // At this stage should opt for the first A
  EXPECT_EQ(sut.getNextNoteIndex(), 3);
  likelihoodGetter->setOnly(59, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 4);
  // At that stage knows it was the final A-B-C being played.
  likelihoodGetter->setOnly(60, 1.f);
}

TEST(DefaultMelodyRecognizer, skippedNote) {
  auto likelihoodGetter = new ObservationLikelihoodGetterFake(intervalSet);
  DefaultMelodyRecognizer sut{
      std::unique_ptr<ObservationLikelihoodGetter>{likelihoodGetter},
      timedMelody};
  likelihoodGetter->setOnly(60, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 1);
  likelihoodGetter->setOnly(59, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 2);
  // likelihoodGetter->setOnly(57, 1.f);
  // EXPECT_THAT(sut.getNextNoteIndex(), Optional(3));
  likelihoodGetter->setOnly(59, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 4);
  likelihoodGetter->setOnly(57, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 5);
  likelihoodGetter->setOnly(59, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), 6);
  likelihoodGetter->setOnly(60, 1.f);
}
} // namespace saint
