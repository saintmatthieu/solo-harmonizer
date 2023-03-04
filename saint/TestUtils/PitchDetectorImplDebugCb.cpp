#include "PitchDetectorImplDebugCb.h"
#include "testUtils.h"

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
  std::array<OlapMetricWriters, 2> olapWriters;
};

OlapMetricWriters makeOlapMetricWriters(int analysisIndex) {
  auto autoCor = testUtils::getWavFileWriter(
      {outDir + "autoCor_" + std::to_string(analysisIndex) + ".wav"});
  auto autoCorMax = testUtils::getWavFileWriter(
      {outDir + "autoCorMax_" + std::to_string(analysisIndex) + ".wav"});
  return {std::move(autoCor), std::move(autoCorMax)};
}
} // namespace

PitchDetectorImplCb getPitchDetectorImplDebugCb() {
  auto metricWriters = std::make_shared<MetricWriters>();
  metricWriters->combinedMax =
      testUtils::getWavFileWriter({outDir + "autoCorMaxMin.wav"});
  metricWriters->olapWriters = {makeOlapMetricWriters(0),
                                makeOlapMetricWriters(1)};
  return [first = true,
          metricWriters](const PitchDetectorImplCbArgs &args) mutable {
    std::vector<float> truncatedXcorr(args.windowSize);
    std::vector<const float *> channels(1);
    std::copy(args.xcor.begin(), args.xcor.begin() + args.windowSize / 2,
              truncatedXcorr.begin());
    std::copy(args.xcor.end() - args.windowSize / 2, args.xcor.end(),
              truncatedXcorr.begin() + args.windowSize / 2);
    truncatedXcorr[std::min((size_t)args.peakIndex,
                            truncatedXcorr.size() - 1)] = 0.f;
    const auto offset = first ? args.windowSize / 2 : 0;
    const auto size = first ? args.windowSize / 2 : args.windowSize;
    first = false;
    channels[0] = truncatedXcorr.data() + offset;
    metricWriters->olapWriters[args.olapAnalIndex]
        .autoCorr->writeFromFloatArrays(channels.data(), 1, size);
    std::vector<float> buffer(size);
    std::fill(buffer.begin(), buffer.end(), args.scaledMax);
    channels[0] = buffer.data();
    metricWriters->olapWriters[args.olapAnalIndex]
        .autoCorrMax->writeFromFloatArrays(channels.data(), 1, size);
    std::fill(buffer.begin(), buffer.begin() + size / 2, args.maxMin);
    metricWriters->combinedMax->writeFromFloatArrays(channels.data(), 1,
                                                     size / 2);
  };
}
} // namespace testUtils
} // namespace saint