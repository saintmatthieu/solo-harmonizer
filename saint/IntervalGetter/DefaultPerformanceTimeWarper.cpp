#include "DefaultPerformanceTimeWarper.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>

namespace saint {
std::unique_ptr<PerformanceTimeWarper> PerformanceTimeWarper::createInstance(
    const std::vector<std::pair<float, std::optional<int>>> &timedNoteNumbers) {
  return std::make_unique<DefaultPerformanceTimeWarper>(timedNoteNumbers);
}

DefaultPerformanceTimeWarper::DefaultPerformanceTimeWarper(
    const std::vector<std::pair<float, std::optional<int>>> &midiFileNns)
    : _fileBreakpoints(midiFileNns) {}

float DefaultPerformanceTimeWarper::getWarpedTime(
    float playheadTime, const std::optional<float> &noteNumber) {

  _updateMeasurements(playheadTime, noteNumber);
  if (noteNumber.has_value() && !_prevWasPitched) {
    const auto hypothesisEndIndices = _getHypothesisEndIndices(playheadTime);
    if (!hypothesisEndIndices.empty()) {
      _updateLastStateChangeMatchTime(playheadTime, hypothesisEndIndices);
    }
  }
  _prevWasPitched = noteNumber.has_value();
  return _lastStateChangeMatchTime + (playheadTime - _playheadTimeByLastChange);
}

void DefaultPerformanceTimeWarper::_updateMeasurements(
    float playheadTime, const std::optional<float> &noteNumber) {

  _measurementTimes.push_back(playheadTime);
  _measuredNns.push_back(noteNumber);

  constexpr auto forgetTimeInCrotchets = 3.f;
  while (_measurementTimes.begin() != _measurementTimes.end() &&
         playheadTime - *_measurementTimes.begin() > forgetTimeInCrotchets) {
    _measurementTimes.erase(_measurementTimes.begin());
    _measuredNns.erase(_measuredNns.begin());
  }
}

std::vector<int>
DefaultPerformanceTimeWarper::_getHypothesisEndIndices(float playheadTime) {

  constexpr auto searchRangeCrotchets = 1.f;
  auto bkptIt = _fileBreakpoints.begin();
  while (bkptIt != _fileBreakpoints.end() &&
         playheadTime - bkptIt->first > searchRangeCrotchets) {
    ++bkptIt;
  }
  // Now we're at the beginning of the search range. Let's collect the file
  // state changes matching ours.
  std::vector<int> hypothesisEndIndices;
  while (bkptIt != _fileBreakpoints.end() &&
         bkptIt->first - playheadTime <= searchRangeCrotchets) {
    if (bkptIt->second.has_value()) {
      hypothesisEndIndices.push_back(
          static_cast<int>(std::distance(_fileBreakpoints.begin(), bkptIt)));
    }
    ++bkptIt;
  }
  return hypothesisEndIndices;
}

void DefaultPerformanceTimeWarper::_updateLastStateChangeMatchTime(
    float playheadTime, const std::vector<int> &hypothesisEndIndices) {

  // Now test hypotheses
  std::vector<float> scores;
  scores.reserve(hypothesisEndIndices.size());
  for (auto hypoEndIndex : hypothesisEndIndices) {
    auto measurementIndex = static_cast<int>(_measurementTimes.size()) - 1;
    auto bkptIndex = hypoEndIndex;
    const auto endBkptIndexTime = _fileBreakpoints[bkptIndex].first;
    std::vector<std::optional<float>> fileNns;
    while (measurementIndex >= 0) {
      const auto measurementTime = _measurementTimes[measurementIndex];
      const auto measurementOffset = playheadTime - measurementTime;
      while (bkptIndex >= 0 &&
             endBkptIndexTime - _fileBreakpoints[bkptIndex].first <
                 measurementOffset) {
        --bkptIndex;
      }
      if (bkptIndex < 0) {
        break;
      }
      const std::optional<float> &pitch = _fileBreakpoints[bkptIndex].second;
      fileNns.push_back(pitch);
      --measurementIndex;
    }
    std::reverse(fileNns.begin(), fileNns.end());
    const auto numPoints = std::min(_measuredNns.size(), fileNns.size());
    auto overlap = 0.f;
    auto numStateMatches = 0;
    for (auto i = 0u; i < numPoints; ++i) {
      if (_measuredNns[i].has_value() == fileNns[i].has_value()) {
        ++numStateMatches;
      }
      if (!_measuredNns[i].has_value() || !fileNns[i].has_value()) {
        continue;
      }
      const auto bottom = std::max(*fileNns[i], *_measuredNns[i]) - 1.f;
      const auto top = std::min(*fileNns[i], *_measuredNns[i]) + 1.f;
      if (top > bottom) {
        overlap += (top - bottom) / 2.f;
      }
    }
    scores.push_back(static_cast<float>(numStateMatches) + overlap);
  }
  const auto winnerIndex = std::distance(
      scores.begin(), std::max_element(scores.begin(), scores.end()));
  const auto maxScore = scores[winnerIndex];
  _lastStateChangeMatchTime =
      _fileBreakpoints[hypothesisEndIndices[winnerIndex]].first;
  _playheadTimeByLastChange = playheadTime;
}
} // namespace saint