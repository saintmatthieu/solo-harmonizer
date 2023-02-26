#include "OnsetDetector.h"

#include <cmath>
#include <limits>
#include <math.h>
#include <numeric>

namespace saint {
namespace {
constexpr auto twoPi = 6.283185307179586f;

int getFftOrder(int windowSize) {
  return static_cast<int>(ceil(log2(windowSize)));
}

int getFftSizeSamples(size_t windowSize) {
  return 1 << getFftOrder(windowSize);
}

int getWindowSizeSamples(int sampleRate) {
  return OnsetDetector::windowSizeMs * sampleRate / 1000;
}

std::vector<float> getAnalysisWindow(int sampleRate) {
  const auto windowSize = getWindowSizeSamples(sampleRate);
  const auto fftSize = getFftSizeSamples(windowSize);
  std::vector<float> window(windowSize);
  const auto freq = twoPi / windowSize;
  // TODO: make sure a rectangular window is tried.
  for (auto i = 0; i < windowSize; ++i) {
    // A Hanning window.
    // For this use case and if there is not need for overlapping windows,
    // a flat-top might work as well.
    // window[i] = 1.f / fftSize;
    window[i] = (1 - cosf(i * freq)) / (2 * fftSize);
  }
  return window;
}

std::vector<float> getFlatTopAnalysisWindow(int sampleRate) {
  const auto windowSize = getWindowSizeSamples(sampleRate);
  const auto firstSearchIndex = std::min(sampleRate / 1500, windowSize);
  std::vector<float> window(windowSize);
  std::fill(window.begin(), window.end(), 1.f);
  const auto freq = twoPi / firstSearchIndex;
  for (auto i = 0; i < firstSearchIndex / 2; ++i) {
    window[i] = (1 - cosf(freq * i)) / 2;
  }
  for (auto i = firstSearchIndex / 2; i < firstSearchIndex; ++i) {
    window[windowSize + i - firstSearchIndex] = (1 - cosf(freq * i)) / 2;
  }
  return window;
}

void applyWindow(const std::vector<float> &window, float *input) {
  for (auto i = 0; i < window.size(); ++i) {
    input[i] *= window[i];
  }
}

void getXCorr(pffft::Fft<float> &fftEngine, float *timeData,
              std::complex<float> *freqData) {
  fftEngine.forward(timeData, freqData);
  for (auto i = 0; i < fftEngine.getSpectrumSize(); ++i) {
    auto &X = freqData[i];
    X *= std::complex<float>{X.real(), -X.imag()};
  }
  fftEngine.inverse(freqData, timeData);
  const auto normalizer = 1.f / timeData[0];
  for (auto i = 0u; i < fftEngine.getLength(); ++i) {
    timeData[i] *= normalizer;
  }
}

std::vector<float> getWindowXCorr(pffft::Fft<float> &fftEngine,
                                  const std::vector<float> &window) {
  Aligned<std::vector<float>> xcorrAligned;
  auto &xcorr = xcorrAligned.value;
  xcorr.resize((fftEngine.getLength()));
  std::copy(window.begin(), window.end(), xcorr.begin());
  std::fill(xcorr.begin() + window.size(), xcorr.end(), 0.f);
  std::vector<std::complex<float>> freqData(fftEngine.getSpectrumSize());
  getXCorr(fftEngine, xcorr.data(), freqData.data());
  return xcorr;
};
} // namespace

OnsetDetector::OnsetDetector(int sampleRate,
                             std::optional<OnXcorReady> onXcorReady)
    : _onXcorReady(std::move(onXcorReady)),
      _window(getAnalysisWindow(sampleRate)),
      _fftSizeSamples(getFftSizeSamples(_window.size())), _firstSearchIndex(
                                                              sampleRate / 1500 /*electric guitar played on high E, 24th fret is around 1328Hz */),
      _lastSearchIndex(std::min(_fftSizeSamples / 2, sampleRate / 83)),
      _fftEngine(_fftSizeSamples),
      _windowXCorr(getWindowXCorr(_fftEngine, _window)) {

  // Fill the first ring buffer with half the window size of zeros.
  std::vector<float> zeros(_window.size() / 2);
  std::fill(zeros.begin(), zeros.end(), 0.f);
  _ringBuffers[0].writeBuff(zeros.data(), zeros.size());

  _timeData.value.resize(_fftSizeSamples);
  _freqData.value.resize(_fftSizeSamples / 2);
}

bool OnsetDetector::process(const float *audio, int audioSize) {
  _ringBuffers[0].writeBuff(audio, audioSize);
  _ringBuffers[1].writeBuff(audio, audioSize);
  while (_ringBuffers[_ringBufferIndex].readAvailable() >= _window.size()) {
    auto timeData = _timeData.value.data();
    _ringBuffers[_ringBufferIndex].readBuff(timeData, _window.size());
    std::fill(timeData + _window.size(), timeData + _fftSizeSamples, 0.f);
    applyWindow(_window, timeData);
    getXCorr(_fftEngine, timeData, _freqData.value.data());
    auto max = 0.f;
    auto maxIndex = 0;
    for (auto i = _firstSearchIndex; i < _lastSearchIndex; ++i) {
      if (timeData[i] > max) {
        max = timeData[i];
        maxIndex = i;
      }
    }
    max /= _windowXCorr[maxIndex];
    if (_onXcorReady) {
      (*_onXcorReady)(_timeData.value, _window.size(), maxIndex,
                      _ringBufferIndex, max);
    }
    _ringBufferIndex = (_ringBufferIndex + 1) % _ringBuffers.size();
  }
  return false;
}
} // namespace saint
