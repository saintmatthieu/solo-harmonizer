#include "OnsetDetector.h"

#include <math.h>

namespace saint {
namespace {
// An electric guitar's lower E is normally 83Hz.
// For a spectrum using a 1st-order cosine window, each sinusoid lobe spans 2
// bins. That requires a window of ~2/83s=25ms. Let's squeeze this down a little
// to 20ms.
constexpr auto fftSizeMs = 20;

std::vector<float> getAnalysisWindow(int sampleRate) {
  const auto windowSize = fftSizeMs * sampleRate / 1000;
  std::vector<float> window(windowSize);
  const auto freq = TWOPI / windowSize;
  for (auto i = 0; i < windowSize; ++i) {
    // A Hanning window.
    // For this use case and if there is not need for overlapping windows,
    // a flat-top might work as well.
    window[i] = (1 - cosf(i * freq)) / 2;
  }
  return window;
}

int getFftOrder(int windowSize) {
  return static_cast<int>(ceil(log2(windowSize)));
}

int getFftSizeSamples(int sampleRate) {
  const auto windowSize = fftSizeMs * sampleRate / 1000;
  return 1 << getFftOrder(windowSize);
}

void applyWindow(const std::vector<float> &window, float *input) {
  for (auto i = 0; i < window.size(); ++i) {
    input[i] *= window[i];
  }
}
} // namespace

OnsetDetector::OnsetDetector(int sampleRate)
    : _window(getAnalysisWindow(sampleRate)),
      _fftSizeSamples(getFftSizeSamples(sampleRate)),
      _fftEngine(_fftSizeSamples), _fftData(_fftSizeSamples / 2) {
  _readBuffer.array.fill(0.f);
  constexpr auto odfType =
      ODS_ODF_RCOMPLEX; // May have to experiment with other
                        // onset-detection function types.
  // "The number of past frames that will be used for median calculation during
  // triggering" Just setting this to 3 past frames - halfway educated guess,
  // might well need tuning.
  const auto medspan = 11;
  const auto memneeded = onsetsds_memneeded(odfType, _fftSizeSamples, medspan);
  _onsetsDSBuffer.resize(memneeded);

  // The ODS_FFT_SC3_COMPLEX seems to hint that a complex input is expected,
  // but debugging showed that it was indeed interpretting the FFT data like
  // PFFFT produces it.
  constexpr auto fftFormat = ODS_FFT_SC3_COMPLEX;
  onsetsds_init(&_onsetsDS, reinterpret_cast<float *>(_onsetsDSBuffer.data()),
                fftFormat, odfType, _fftSizeSamples, medspan, sampleRate);
}

bool OnsetDetector::process(const float *audio, int audioSize) {
  _ringBuffers[0].writeBuff(audio, audioSize);
  _ringBuffers[1].writeBuff(audio, audioSize);
  const auto halfWindowSize = static_cast<int>(_window.size() / 2);
  if (_numSamplesRemoved < halfWindowSize) {
    const auto numSamplesToRemove =
        std::min(audioSize, halfWindowSize - _numSamplesRemoved);
    _ringBuffers[1].remove(numSamplesToRemove);
    _numSamplesRemoved += numSamplesToRemove;
  }
  auto hasOnset = false;
  while (_ringBuffers[_ringBufferIndex].readAvailable() >= _window.size()) {
    auto readData = _readBuffer.array.data();
    _ringBuffers[_ringBufferIndex].readBuff(readData, _window.size());
    applyWindow(_window, readData);
    _fftEngine.forward(readData, _fftData.data());
    hasOnset |= onsetsds_process(&_onsetsDS,
                                 reinterpret_cast<float *>(_fftData.data()));
    _ringBufferIndex = ++_ringBufferIndex % _ringBuffers.size();
  }
  return hasOnset;
}
} // namespace saint
