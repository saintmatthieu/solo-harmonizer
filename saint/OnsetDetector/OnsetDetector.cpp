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

std::vector<float> getXCorr(pffft::Fft<float> &fftEngine, const float *data) {
  std::vector<std::complex<float>> freqData(fftEngine.getSpectrumSize());
  std::vector<float> xcorr(fftEngine.getLength());
  fftEngine.forward(data, freqData.data());
  for (auto i = 0; i < fftEngine.getSpectrumSize(); ++i) {
    auto &X = freqData[i];
    X *= std::complex<float>{X.real(), -X.imag()};
  }
  fftEngine.inverse(freqData.data(), xcorr.data());
  const auto normalizer = 1.f / xcorr[0];
  for (auto i = 0u; i < xcorr.size(); ++i) {
    xcorr[i] *= normalizer;
  }
  return xcorr;
};
} // namespace

OnsetDetector::OnsetDetector(int sampleRate)
    : _window(getAnalysisWindow(sampleRate)),
      _fftSizeSamples(getFftSizeSamples(_window.size())), _firstSearchIndex(
                                                              sampleRate / 1500 /*electric guitar played on high E, 24th fret is around 1328Hz */),
      _lastSearchIndex(std::min(_fftSizeSamples / 2, sampleRate / 83)),
      _fftEngine(_fftSizeSamples),
      _windowXCorr(getXCorr(_fftEngine, _window.data())) {

  _fftEngine.prepareLength(_fftSizeSamples);
  _timeData.value.resize(_fftSizeSamples);
  std::fill(_timeData.value.begin(), _timeData.value.end(), 0.f);

  _freqData.value.resize(_fftSizeSamples / 2);
  _autoCorrFreqData.value.resize(_fftSizeSamples / 2);
  _autoCorrTimeData.value.resize(_fftSizeSamples);
}

bool OnsetDetector::process(const float *audio, int audioSize) {
  _ringBuffer.writeBuff(audio, audioSize);
  _peakMax = 0;
  while (_ringBuffer.readAvailable() >= _window.size()) {
    auto timeData = _timeData.value.data();
    _ringBuffer.readBuff(timeData, _window.size());
    applyWindow(_window, timeData);
    _fftEngine.forward(timeData, _freqData.value.data());
    for (auto i = 0; i < _fftSizeSamples / 2; ++i) {
      const auto &X = _freqData.value[i];
      _autoCorrFreqData.value[i] = X * std::complex<float>{X.real(), -X.imag()};
    }
    _fftEngine.inverse(_autoCorrFreqData.value.data(),
                       _autoCorrTimeData.value.data());
    const auto normalizer = 1.f / _autoCorrTimeData.value[0];
    for (auto i = 0u; i < _window.size(); ++i) {
      _autoCorrTimeData.value[i] *= normalizer;
      if (_firstSearchIndex <= i && i < _lastSearchIndex &&
          _autoCorrTimeData.value[i] > _peakMax) {
        _peakMax = _autoCorrTimeData.value[i];
        _peakMaxIndex = i;
      }
    }
  }
  _peakMax /= _windowXCorr[_peakMaxIndex];
  return false;
}
} // namespace saint
