#include "DefaultMelodyFollower.h"

#include "gmock/gmock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace saint {

using namespace ::testing;
namespace {
const std::vector<std::pair<float, std::optional<int>>> timedMelody{
    {0.f, {60}}, {1.f, {59}}, {2.f, {57}}, {3.f, {59}},
    {4.f, {57}}, {5.f, {59}}, {6.f, {60}},
};

const std::vector<int> melody{60, 59, 57, 59, 57, 59, 60};
} // namespace

class ObservationLikelihoodGetterFake : public ObservationLikelihoodGetter {
public:
  void addObservationSample(float, float) override {}
  std::unordered_map<int, float> consumeObservationSamples() override {
    return observationLikelihoods;
  }
  void setOnly(int nn, float prob) {
    observationLikelihoods.clear();
    observationLikelihoods[nn] = prob;
  }
  bool hasObservationSamples() const override {
    return observationLikelihoods.size() > 0u;
  }
  std::unordered_map<int, float> observationLikelihoods;
};

TEST(DefaultMelodyFollower, getNextNoteIndex_returnsNulloptByDefault) {
  ObservationLikelihoodGetterFake likelihoodGetter;
  DefaultMelodyFollower sut{likelihoodGetter, timedMelody};
  EXPECT_EQ(sut.getNextNoteIndex(), std::nullopt);
}

TEST(DefaultMelodyFollower, easy) {
  ObservationLikelihoodGetterFake likelihoodGetter;
  DefaultMelodyFollower sut{likelihoodGetter, timedMelody};
  likelihoodGetter.setOnly(60, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(1));
  likelihoodGetter.setOnly(59, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(2));
  likelihoodGetter.setOnly(57, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(3));
  likelihoodGetter.setOnly(59, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(4));
  likelihoodGetter.setOnly(57, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(5));
  likelihoodGetter.setOnly(59, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(6));
  likelihoodGetter.setOnly(60, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), std::nullopt);
}

TEST(DefaultMelodyFollower, beginFromTheMiddle) {
  ObservationLikelihoodGetterFake likelihoodGetter;
  DefaultMelodyFollower sut{likelihoodGetter, timedMelody};
  likelihoodGetter.setOnly(57, 1.f);
  // At this stage should opt for the first A
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(3));
  likelihoodGetter.setOnly(59, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(4));
  // At that stage knows it was the final A-B-C being played.
  likelihoodGetter.setOnly(60, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), std::nullopt);
}

TEST(DefaultMelodyFollower, skippedNote) {
  ObservationLikelihoodGetterFake likelihoodGetter;
  DefaultMelodyFollower sut{likelihoodGetter, timedMelody};
  likelihoodGetter.setOnly(60, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(1));
  likelihoodGetter.setOnly(59, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(2));
  // likelihoodGetter.setOnly(57, 1.f);
  // EXPECT_THAT(sut.getNextNoteIndex(), Optional(3));
  likelihoodGetter.setOnly(59, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(4));
  likelihoodGetter.setOnly(57, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(5));
  likelihoodGetter.setOnly(59, 1.f);
  EXPECT_THAT(sut.getNextNoteIndex(), Optional(6));
  likelihoodGetter.setOnly(60, 1.f);
  EXPECT_EQ(sut.getNextNoteIndex(), std::nullopt);
}

TEST(DefaultMelodyFollowerHelper, getIndexOfLastSnippetElement) {
  using Sut = DefaultMelodyFollowerHelper;
  EXPECT_THAT(
      Sut::getIndexOfLastSnippetElement(melody, {60, 59, 59}, std::nullopt),
      Optional(1));
  EXPECT_THAT(
      Sut::getIndexOfLastSnippetElement(melody, {60, 57, 59}, std::nullopt),
      Optional(3));
  EXPECT_THAT(
      Sut::getIndexOfLastSnippetElement(melody, {57, 59, 60}, std::nullopt),
      Optional(6));
}
} // namespace saint
