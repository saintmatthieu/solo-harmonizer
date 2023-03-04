#include "PitchDetectorDebugCb.h"
#include "testUtils.h"

#include <juce_audio_formats/juce_audio_formats.h>

namespace saint {
namespace testUtils {
namespace {
const std::string outDir = "C:/Users/saint/Downloads/";
struct OlapMetricWriters {
  std::unique_ptr<juce::AudioFormatWriter> autoCorr;
  std::unique_ptr<juce::AudioFormatWriter> autoCorrMax;
};

struct MetricWriters {
  std::unique_ptr<juce::AudioFormatWriter> combinedMax;
  std::unique_ptr<juce::AudioFormatWriter> detectedPitch;
  std::array<OlapMetricWriters, 2> olapWriters;
};

OlapMetricWriters makeOlapMetricWriters(int analysisIndex) {
  auto autoCor = testUtils::getWavFileWriter(
      {outDir + "pd_autoCor_" + std::to_string(analysisIndex) + ".wav"});
  auto autoCorMax = testUtils::getWavFileWriter(
      {outDir + "pd_autoCorMax_" + std::to_string(analysisIndex) + ".wav"});
  return {std::move(autoCor), std::move(autoCorMax)};
}
} // namespace

PitchDetectorDebugCb getPitchDetectorDebugCb() {
  auto metricWriters = std::make_shared<MetricWriters>();
  metricWriters->combinedMax =
      testUtils::getWavFileWriter({outDir + "pd_autoCorMaxMin.wav"});
  metricWriters->detectedPitch =
      testUtils::getWavFileWriter({outDir + "pd_detectedPitch.wav"});
  metricWriters->olapWriters = {makeOlapMetricWriters(0),
                                makeOlapMetricWriters(1)};
  return [first = true,
          metricWriters](const PitchDetectorDebugCbArgs &args) mutable {
    for (const auto anal : args.anal) {
      std::vector<float> truncatedXcorr(anal.windowSize);
      std::vector<const float *> channels(1);
      std::copy(anal.xcor.begin(), anal.xcor.begin() + anal.windowSize / 2,
                truncatedXcorr.begin());
      std::copy(anal.xcor.end() - anal.windowSize / 2, anal.xcor.end(),
                truncatedXcorr.begin() + anal.windowSize / 2);
      truncatedXcorr[std::min((size_t)anal.peakIndex,
                              truncatedXcorr.size() - 1)] = 0.f;
      const auto offset = first ? anal.windowSize / 2 : 0;
      const auto size = first ? anal.windowSize / 2 : anal.windowSize;
      first = false;
      channels[0] = truncatedXcorr.data() + offset;
      metricWriters->olapWriters[anal.olapAnalIndex]
          .autoCorr->writeFromFloatArrays(channels.data(), 1, size);
      std::vector<float> buffer(size);
      std::fill(buffer.begin(), buffer.end(), anal.scaledMax);
      channels[0] = buffer.data();
      metricWriters->olapWriters[anal.olapAnalIndex]
          .autoCorrMax->writeFromFloatArrays(channels.data(), 1, size);
      std::fill(buffer.begin(), buffer.begin() + size / 2, anal.maxMin);
      metricWriters->combinedMax->writeFromFloatArrays(channels.data(), 1,
                                                       size / 2);
    }
    std::vector<float> detectedPitchV(args.blockSize);
    const auto detectedPitch =
        args.detectedPitch.has_value() ? *args.detectedPitch : 0.f;
    std::fill(detectedPitchV.begin(), detectedPitchV.end(),
              detectedPitch / 1000);
    std::vector<const float *> channels(1);
    channels[0] = detectedPitchV.data();
    metricWriters->detectedPitch->writeFromFloatArrays(channels.data(), 1,
                                                       args.blockSize);
  };
}
} // namespace testUtils
} // namespace saint