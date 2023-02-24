#include "OnsetDetector.h"

#include <cmath>
#include <limits>
#include <math.h>
#include <numeric>

namespace saint {
namespace {
// An electric guitar's lower E is normally 83Hz.
// For a spectrum using a 1st-order cosine window, each sinusoid lobe spans 2
// bins. That requires a window of ~2/83s=25ms. Let's squeeze this down a little
// to 20ms.
constexpr auto fftSizeMs = 20;
constexpr auto twoPi = 6.283185307179586f;
constexpr auto windowSize = 512; // fftSizeMs * sampleRate / 1000;
constexpr auto conj = std::complex<float>{1, -1}; // complex conjugate unit

int getFftOrder(int windowSize) {
  return static_cast<int>(ceil(log2(windowSize)));
}

int getFftSizeSamples(size_t windowSize) {
  return 1 << getFftOrder(windowSize);
}

std::vector<float> getAnalysisWindow(int sampleRate) {
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

std::vector<float> getXCorrVector(pffft::Fft<float> &fftEngine,
                                  const std::vector<float> &timeData) {
  Aligned<std::vector<float>> xcorrAligned;
  auto &xcorr = xcorrAligned.value;
  xcorr.resize((fftEngine.getLength()));
  std::copy(timeData.begin(), timeData.end(), xcorr.begin());
  std::fill(xcorr.begin() + timeData.size(), xcorr.end(), 0.f);
  std::vector<std::complex<float>> freqData(fftEngine.getSpectrumSize());
  getXCorr(fftEngine, xcorr.data(), freqData.data());
  return xcorr;
};
} // namespace

OnsetDetector::OnsetDetector(int sampleRate)
    : _window(getAnalysisWindow(sampleRate)),
      _fftSizeSamples(getFftSizeSamples(_window.size())), _firstSearchIndex(
                                                              sampleRate / 1500 /*electric guitar played on high E, 24th fret is around 1328Hz */),
      _lastSearchIndex(std::min(_fftSizeSamples / 2, sampleRate / 83)),
      _fftEngine(_fftSizeSamples),
      _windowXCorr(getXCorrVector(_fftEngine, _window)) {

  _fftEngine.prepareLength(_fftSizeSamples);
  _timeData.value.resize(_fftSizeSamples);
  // Make sure that zero-padding tail is zero.
  std::fill(_timeData.value.begin(), _timeData.value.end(), 0.f);
  _freqData.value.resize(_fftSizeSamples / 2);
}

bool OnsetDetector::process(const float *audio, int audioSize) {
  _ringBuffer.writeBuff(audio, audioSize);
  _peakMax = 0;
  while (_ringBuffer.readAvailable() >= _window.size()) {
    auto timeData = _timeData.value.data();
    _ringBuffer.readBuff(timeData, _window.size());
    applyWindow(_window, timeData);
    getXCorr(_fftEngine, timeData, _freqData.value.data());
    for (auto i = 0u; i < _window.size(); ++i) {
      if (_firstSearchIndex <= i && i < _lastSearchIndex &&
          timeData[i] > _peakMax) {
        _peakMax = timeData[i];
        _peakMaxIndex = i;
      }
    }
  }
  _peakMax /= _windowXCorr[_peakMaxIndex];
  return false;
}
} // namespace saint
