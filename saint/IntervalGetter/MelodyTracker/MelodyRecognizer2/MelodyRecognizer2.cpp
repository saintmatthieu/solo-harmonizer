#include "MelodyRecognizer2.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <limits>
#include <numeric>
#include <unordered_set>

namespace saint {

using Melody = MelodyRecognizer2::Melody;
using MotiveInstance = MelodyRecognizer2::MotiveInstance;
using MotiveInvariants = MelodyRecognizer2::MotiveInvariants;
using TableRow = MelodyRecognizer2::TableRow;

namespace {
float getAverage(const std::vector<float> &x) {
  const auto N = static_cast<float>(x.size());
  return std::accumulate(x.begin(), x.end(), 0.f) / N;
}

float getVariance(const std::vector<float> &x, float avg) {
  return std::accumulate(x.begin(), x.end(), 0.f,
                         [avg](float acc, float x) {
                           return acc + (x - avg) * (x - avg);
                         }) /
         static_cast<float>(x.size());
}

std::pair<float /*avg*/, float /*likelihood*/>
getIntervalLikelihood(const std::vector<int> &noteNumberSequence,
                      const std::vector<std::vector<float>> &experiments) {
  assert(experiments.size() == noteNumberSequence.size());
  std::vector<float> lessRefNoteNumbers;
  for (auto e = 0u; e < experiments.size(); ++e) {
    for (auto observation : experiments[e]) {
      lessRefNoteNumbers.push_back(observation - noteNumberSequence[e]);
    }
  }
  const auto avg = getAverage(lessRefNoteNumbers);
  const auto variance = getVariance(lessRefNoteNumbers, avg);
  // Variance here has unit semitone squared. E.g.
  //    Reference pitch : [60, 60]
  //    Actual pitch : [63, 65]
  //    Fit : [59, 61] => variance = 1
  // We penalize this giving a probability of 1/2.
  return {avg, std::powf(2.f, -variance)};
}

std::pair<float /*avg*/, float /*likelihood*/>
getDurationLikelihood(const std::vector<float> &inputLogDurations,
                      const std::vector<float> &experimentLogDurations) {
  assert(inputLogDurations.size() == experimentLogDurations.size());
  const auto N = experimentLogDurations.size();
  std::vector<float> timeRatios(N);
  for (auto i = 0u; i < N; ++i) {
    timeRatios[i] = experimentLogDurations[i] - inputLogDurations[i];
  }
  const auto avg = getAverage(timeRatios);
  const auto variance = getVariance(timeRatios, avg);
  // Variance here has unit "time octave" squared. E.g.
  //    Reference duration : [1, 1] (sec) -> log -> [0, 0]
  //    Actual duration : [2, 8] (sec) -> log -> [1, 3]
  //    Fit (log) : [-1, 1] => variance = 1.
  // We penalize this giving a probability of 1/2. It may not seem much, but we
  // should probably be less severe with duration as with pitch, people don't
  // necessarily care about muting in time.
  return {avg, std::powf(2.f, -variance)};
}

std::vector<int> getNoteNumbers(const Melody &melody) {
  std::vector<int> out(melody.size());
  std::transform(
      melody.begin(), melody.end(), out.begin(),
      [](const std::pair<float, int> &entry) { return entry.second; });
  return out;
}

std::vector<float> getDurations(const Melody &melody) {
  std::vector<float> out(melody.size());
  std::transform(
      melody.begin(), melody.end(), out.begin(),
      [](const std::pair<float, int> &entry) { return entry.first; });
  return out;
}

constexpr auto numConsideredExperiments = 2u;

template <typename T>
void reorder(std::vector<T> &v, const std::vector<size_t> &order) {
  // https://stackoverflow.com/questions/838384/reorder-vector-using-a-vector-of-indices
  for (int s = 1, d; s < order.size(); ++s) {
    for (d = order[s]; d < s; d = order[d]) {
    }
    if (d == s) {
      while (d = order[d], d != s) {
        std::swap(v[s], v[d]);
      }
    }
  }
}

std::vector<TableRow> makeTable(const Melody &melody) {
  std::map<Melody, std::unordered_map<size_t, MotiveInstance>> motiveInstances;
  for (auto i = 0u; i < melody.size() - numConsideredExperiments + 1u; ++i) {
    Melody shifted;
    shifted.reserve(numConsideredExperiments);
    const auto firstDuration = melody[i].first;
    const auto firstPitch = melody[i].second;
    std::transform(
        melody.begin() + i, melody.begin() + i + numConsideredExperiments,
        std::back_inserter(shifted),
        [&](const std::pair<float, int> &note) -> std::pair<float, int> {
          return {note.first - firstDuration, note.second - firstPitch};
        });
    motiveInstances[shifted][i] = {firstPitch, firstDuration};
  }
  std::vector<TableRow> asTable;
  std::vector<size_t> beginIndices;
  for (const auto &entry : motiveInstances) {
    const auto &[motive, instances] = entry;
    const auto sharedMotive = std::make_shared<Melody>(motive);
    for (const auto &entry : instances) {
      const auto &[beginIndex, instance] = entry;
      asTable.emplace_back(sharedMotive, instance.firstNoteNumber,
                           instance.firstDuration);
      beginIndices.emplace_back(beginIndex);
    }
  }
  reorder(asTable, beginIndices);
  return asTable;
}

MotiveInvariants getMotiveBeginInstances(const Melody &melody) {
  std::map<Melody, std::unordered_map<size_t, MotiveInstance>> motiveInstances;
  for (auto i = 0u; i < melody.size() - numConsideredExperiments + 1u; ++i) {
    std::vector<std::pair<float, int>> shifted(numConsideredExperiments);
    const auto firstDuration = melody[i].first;
    const auto firstPitch = melody[i].second;
    std::transform(
        melody.begin() + i, melody.begin() + i + numConsideredExperiments,
        shifted.begin(),
        [&](const std::pair<float, int> &note) -> std::pair<float, int> {
          return {note.first - firstDuration, note.second - firstPitch};
        });
    motiveInstances[shifted][i] = {firstPitch, firstDuration};
  }
  MotiveInvariants asVector(motiveInstances.size());
  std::transform(motiveInstances.begin(), motiveInstances.end(),
                 asVector.begin(), [](const auto &entry) { return entry; });
  return asVector;
}

float getReferenceDuration(const Melody &melody) {
  // A pitch class p is a log2(f/f0) operation. For our purpose it is convenient
  // to have durations in similar form. As duration reference we choose the
  // least duration. Ideally this is quantized and we can get nice round values
  // when applying our log2 formula.
  return std::log2f(std::min_element(melody.begin(), melody.end(),
                                     [](const auto &a, const auto &b) {
                                       return a.first < b.first;
                                     })
                        ->first);
}

Melody convertDurationsToLog(Melody melody, float referenceDuration) {
  std::transform(
      melody.begin(), melody.end(), melody.begin(),
      [referenceDuration](const auto &entry) -> std::pair<float, int> {
        return {std::log2f(entry.first) - referenceDuration, entry.second};
      });
  return melody;
}

float getPitchTranspositionLikelihood(float a, float b) {
  // We tolerate intonation errors but no on-the-fly transpositions. For now
  // accept anything within 1 semitone.
  // Todo: tune
  return std::abs(a - b) > .5f ? -std::numeric_limits<float>::infinity() : 0.f;
}

float getDurationTranspositionLikelihood(float a, float b) {
  // From one note to the next, tolerate an acceleration by a factor of 2, but
  // nothing else for now. Todo: tune
  return std::abs(a - b) > .5f ? -std::numeric_limits<float>::infinity() : 0.f;
}
} // namespace

MelodyRecognizer2::MelodyRecognizer2(Melody melody)
    : _referenceDuration(getReferenceDuration(melody)),
      _table(makeTable(melody)),
      _motives(getMotiveBeginInstances(
          convertDurationsToLog(std::move(melody), _referenceDuration))) {}

std::optional<size_t> MelodyRecognizer2::beginNewNote(int tickCounter) {

  static std::ofstream log("C:/Users/saint/downloads/log.txt");

  if (!_currentExperiment.has_value()) {
    log << "nullopt" << std::endl;
    return std::nullopt;
  }
  _lastExperiments.push_back(*_currentExperiment); // todo optimize
  _currentExperiment.reset();

  const auto experimentLogDuration = std::log2f(
      static_cast<float>(tickCounter - _prevNoteonTick) - _referenceDuration);
  _prevNoteonTick = tickCounter;

  _lastExperimentsLogDurations.push_back(experimentLogDuration);
  if (_lastExperiments.size() < numConsideredExperiments) {
    log << "nullopt" << std::endl;
    return std::nullopt;
  } else if (_lastExperiments.size() > numConsideredExperiments) {
    _lastExperiments.erase(_lastExperiments.begin());
    _lastExperimentsLogDurations.erase(_lastExperimentsLogDurations.begin());
  }

  std::unordered_map<std::shared_ptr<Melody>, const TableRow *> referenceRows;
  for (const auto &row : _table) {
    if (referenceRows.count(row.motive) == 0) {
      const auto motiveNoteNumbers = getNoteNumbers(*row.motive);
      const auto motiveDurations = getDurations(*row.motive);
      const auto [pitchClassErrorAvg, intervalLlh] =
          getIntervalLikelihood(motiveNoteNumbers, _lastExperiments);
      const auto [durationErrorAvg, durationLlh] =
          getDurationLikelihood(motiveDurations, _lastExperimentsLogDurations);
      row.currProb =
          std::make_shared<float>(std::logf(intervalLlh * durationLlh));
      row.pitchClassErrorAvg = std::make_shared<float>(pitchClassErrorAvg);
      row.durationErrorAvg = std::make_shared<float>(durationErrorAvg);
      referenceRows[row.motive] = &row;
    } else {
      const TableRow *ref = referenceRows.at(row.motive);
      row.currProb = ref->currProb;
      row.pitchClassErrorAvg = ref->pitchClassErrorAvg;
      row.durationErrorAvg = ref->durationErrorAvg;
    }
    row.currTranspositions.emplace(*row.pitchClassErrorAvg -
                                       row.firstNoteNumber,
                                   *row.durationErrorAvg - row.firstDuration);
  }

  if (_table[0].prevProb == nullptr) {
    updateTablePrevFields();
    return std::nullopt;
  }

  std::vector<float> probs(_table.size() - 1u);
  for (auto i = 0u; i < probs.size(); ++i) {
    const auto &a = _table[i];
    const auto &b = _table[i + 1u];
    probs[i] = *a.prevProb + *b.currProb +
               getPitchTranspositionLikelihood(a.prevTranspositions->first,
                                               b.currTranspositions->first) +
               getDurationTranspositionLikelihood(a.prevTranspositions->second,
                                                  b.currTranspositions->second);
  }
  const auto maxIndex = std::distance(
      probs.begin(), std::max_element(probs.begin(), probs.end()));

  updateTablePrevFields();

  if (probs[maxIndex] < -1.f) {
    return std::nullopt;
  } else {
    return maxIndex + numConsideredExperiments - 1u;
  }
}

void MelodyRecognizer2::addPitchMeasurement(float pc) {
  if (!_currentExperiment.has_value()) {
    _currentExperiment = std::vector<float>{};
  }
  _currentExperiment->push_back(pc);
}

void MelodyRecognizer2::updateTablePrevFields() {
  for (const auto &row : _table) {
    row.prevProb = row.currProb;
    row.prevTranspositions = row.currTranspositions;
  }
}

MelodyRecognizer2::TableRow::TableRow(std::shared_ptr<Melody> motive,
                                      int firstNoteNumber, float firstDuration)
    : motive(std::move(motive)), firstNoteNumber(firstNoteNumber),
      firstDuration(firstDuration) {}

} // namespace saint