#include "PitchDetectorImpl.h"
#include "PitchDetectorDebugCb.h"
#include "Utils.h"

#include <cassert>
#include <cmath>
#include <limits>
#include <math.h>
#include <numeric>
#include <optional>

namespace saint {

std::unique_ptr<PitchDetector> PitchDetector::createInstance(
    int sampleRate, const std::optional<float> &leastFrequencyToDetect) {
  const auto debug =
      utils::getEnvironmentVariableAsBool("SAINT_DEBUG_PITCHDETECTOR");
  if (debug && utils::isDebugBuild()) {
    return std::make_unique<PitchDetectorImpl>(
        sampleRate, leastFrequencyToDetect,
        testUtils::getPitchDetectorDebugCb());
  } else {
    return std::make_unique<PitchDetectorImpl>(
        sampleRate, leastFrequencyToDetect, std::nullopt);
  }
}

namespace {
constexpr auto twoPi = 6.283185307179586f;
constexpr auto cutoffFreq = 1500;

int getFftOrder(int windowSize) {
  return static_cast<int>(ceilf(log2f((float)windowSize)));
}

int getFftSizeSamples(int windowSize) { return 1 << getFftOrder(windowSize); }

int getWindowSizeSamples(int sampleRate,
                         const std::optional<float> &leastFrequencyToDetect) {
  // If not provided, use the lower open-E of a guitar.
  const auto freq =
      leastFrequencyToDetect.has_value() ? *leastFrequencyToDetect : 83.f;

  // 3.3 times the fundamental period. More and that's unnecessary delay, less
  // and the detection becomes inaccurate - at least with this autocorrelation
  // method. A spectral-domain method might need less than this, since
  // autocorrelation requires there to be at least two periods within the
  // window, against 1 for a spectrum reading.
  const auto windowSizeMs = 1000 * 3.5 / freq;
  return windowSizeMs * sampleRate / 1000;
}

std::vector<float> getAnalysisWindow(int windowSize) {
  std::vector<float> window((size_t)windowSize);
  const auto freq = twoPi / (float)windowSize;
  // TODO: make sure a rectangular window is tried.
  for (auto i = 0u; i < windowSize; ++i) {
    // A Hanning window.
    // For this use case and if there is not need for overlapping windows,
    // a flat-top might work as well.
    // window[i] = 1.f / fftSize;
    window[i] = (1 - cosf(i * freq)) / 2;
  }
  return window;
}

void applyWindow(const std::vector<float> &window, std::vector<float> &input) {
  for (auto i = 0u; i < window.size(); ++i) {
    input[i] *= window[i];
  }
}

void getXCorr(pffft::Fft<float> &fft, std::vector<float> &time,
              const std::vector<float> &lpWindow) {
  Aligned<std::vector<std::complex<float>>> freq;
  freq.value.resize(fft.getSpectrumSize());
  auto freqData = freq.value.data();
  auto timeData = time.data();
  fft.forward(timeData, freqData);
  for (auto i = 0; i < lpWindow.size(); ++i) {
    auto &X = freqData[i];
    X *= lpWindow[i] * std::complex<float>{X.real(), -X.imag()};
  }
  std::fill(freqData + lpWindow.size(), freqData + fft.getSpectrumSize(), 0.f);
  fft.inverse(freqData, timeData);
  const auto normalizer = 1.f / timeData[0];
  for (auto i = 0u; i < fft.getLength(); ++i) {
    timeData[i] *= normalizer;
  }
}

std::vector<float> getLpWindow(int sampleRate, int fftSize) {
  std::vector<float> window(fftSize / 2);
  const int cutoffBin =
      std::min(fftSize / 2, fftSize * cutoffFreq / sampleRate);
  const int rollOffSize = fftSize * 200 / sampleRate;
  std::fill(window.begin(), window.begin() + cutoffBin, 1.f);
  for (auto i = 0; i < rollOffSize && cutoffBin + rollOffSize < fftSize / 2;
       ++i) {
    window[cutoffBin + i] = 1.f - i / static_cast<float>(rollOffSize);
  }
  std::fill(window.begin() + cutoffBin + rollOffSize, window.end(), 0.f);
  return window;
}

std::vector<float> getWindowXCorr(pffft::Fft<float> &fftEngine,
                                  const std::vector<float> &window,
                                  const std::vector<float> &lpWindow) {
  Aligned<std::vector<float>> xcorrAligned;
  auto &xcorr = xcorrAligned.value;
  xcorr.resize((fftEngine.getLength()));
  std::copy(window.begin(), window.end(), xcorr.begin());
  std::fill(xcorr.begin() + window.size(), xcorr.end(), 0.f);
  getXCorr(fftEngine, xcorr, lpWindow);
  return xcorr;
}
} // namespace

PitchDetectorImpl::PitchDetectorImpl(
    int sampleRate, const std::optional<float> &leastFrequencyToDetect,
    std::optional<testUtils::PitchDetectorDebugCb> debugCb)
    : _sampleRate(sampleRate), _debugCb(std::move(debugCb)),
      _window(getAnalysisWindow(
          getWindowSizeSamples(sampleRate, leastFrequencyToDetect))),
      _fwdFft(_fftSize), _lpWindow(getLpWindow(sampleRate, _fftSize)),
      _fftSize(getFftSizeSamples(static_cast<int>(_window.size()))),
      _lastSearchIndex(
          std::min(_fftSize / 2, static_cast<int>(sampleRate / 70))),
      _windowXcor(getWindowXCorr(_fwdFft, _window, _lpWindow)) {

  // Fill the first ring buffer with half the window size of zeros.
  std::vector<float> zeros(_window.size() / 2);
  std::fill(zeros.begin(), zeros.end(), 0.f);
  _ringBuffers[0].writeBuff(zeros.data(), zeros.size());
}

std::optional<float> PitchDetectorImpl::process(const float *audio,
                                                int audioSize) {
  _ringBuffers[0].writeBuff(audio, audioSize);
  _ringBuffers[1].writeBuff(audio, audioSize);
  std::vector<testUtils::PitchDetectorFftAnal> analyses;
  while (_ringBuffers[_ringBufferIndex].readAvailable() >= _window.size()) {
    std::vector<float> time(_fftSize);
    _ringBuffers[_ringBufferIndex].readBuff(time.data(), _window.size());
    std::fill(time.begin() + _window.size(), time.begin() + _fftSize, 0.f);
    applyWindow(_window, time);
    getXCorr(_fwdFft, time, _lpWindow);
    auto &max = _maxima[_ringBufferIndex] = 0;
    auto maxIndex = 0;
    auto wentNegative = false;
    for (auto i = 0; i < _lastSearchIndex; ++i) {
      wentNegative |= time[i] < 0;
      if (wentNegative && time[i] > max) {
        max = time[i];
        maxIndex = i;
      }
    }
    max /= _windowXcor[maxIndex];
    if (_debugCb) {
      testUtils::PitchDetectorFftAnal analysis;
      analysis.xcor = time;
      analysis.windowSize = _window.size();
      analysis.olapAnalIndex = _ringBufferIndex;
      analysis.peakIndex = maxIndex;
      analysis.scaledMax = max;
      analysis.maxMin = std::min(_maxima[0], _maxima[1]);
      analyses.push_back(analysis);
    }
    _ringBufferIndex = (_ringBufferIndex + 1) % _ringBuffers.size();
    if (max > 0.9) {
      _detectedPitch = _sampleRate / maxIndex;
    } else {
      _detectedPitch.reset();
    }
  }
  if (_debugCb) {
    (*_debugCb)({analyses, _detectedPitch, audioSize});
  }
  return _detectedPitch;
}
} // namespace saint
