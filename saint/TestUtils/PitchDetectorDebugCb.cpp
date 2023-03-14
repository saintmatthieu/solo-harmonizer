#include "PitchDetectorDebugCb.h"
#include "WavFileWriter.h"
#include "testUtils.h"

namespace saint {
namespace testUtils {

namespace fs = std::filesystem;

namespace {
struct OlapMetricWriters {
  std::unique_ptr<WavFileWriter> autoCorr;
  std::unique_ptr<WavFileWriter> autoCorrMax;
};

struct MetricWriters {
  std::unique_ptr<WavFileWriter> combinedMax;
  std::unique_ptr<WavFileWriter> detectedPitch;
  std::array<OlapMetricWriters, 2> olapWriters;
};

OlapMetricWriters makeOlapMetricWriters(int analysisIndex) {
  auto autoCor = std::make_unique<WavFileWriter>(fs::path{
      getOutDir() + "pd_autoCor_" + std::to_string(analysisIndex) + ".wav"});
  auto autoCorMax = std::make_unique<WavFileWriter>(fs::path{
      getOutDir() + "pd_autoCorMax_" + std::to_string(analysisIndex) + ".wav"});
  return {std::move(autoCor), std::move(autoCorMax)};
}
} // namespace

PitchDetectorDebugCb getPitchDetectorDebugCb() {
  auto metricWriters = std::make_shared<MetricWriters>();
  metricWriters->combinedMax = std::make_unique<WavFileWriter>(
      fs::path{getOutDir() + "pd_autoCorMaxMin.wav"});
  metricWriters->detectedPitch = std::make_unique<WavFileWriter>(
      fs::path{getOutDir() + "pd_detectedPitch.wav"});
  metricWriters->olapWriters = {makeOlapMetricWriters(0),
                                makeOlapMetricWriters(1)};
  return [first = true,
          metricWriters](const PitchDetectorDebugCbArgs &args) mutable {
    for (const auto &anal : args.anal) {
      std::vector<float> truncatedXcorr(anal.windowSize);
      std::copy(anal.xcor.begin(), anal.xcor.begin() + anal.windowSize / 2,
                truncatedXcorr.begin());
      std::copy(anal.xcor.end() - anal.windowSize / 2, anal.xcor.end(),
                truncatedXcorr.begin() + anal.windowSize / 2);
      truncatedXcorr[std::min((size_t)anal.peakIndex,
                              truncatedXcorr.size() - 1)] = 0.f;
      const auto offset = first ? anal.windowSize / 2 : 0;
      const auto size = first ? anal.windowSize / 2 : anal.windowSize;
      first = false;
      auto &olapWriter = metricWriters->olapWriters[anal.olapAnalIndex];
      olapWriter.autoCorr->write(truncatedXcorr.data() + offset, size);
      olapWriter.autoCorrMax->write(anal.scaledMax, size);
      metricWriters->combinedMax->write(anal.maxMin, size / 2);
    }
    const auto detectedPitch =
        args.detectedPitch.has_value() ? *args.detectedPitch : 0.f;
    metricWriters->detectedPitch->write(detectedPitch, args.blockSize);
  };
}
} // namespace testUtils
} // namespace saint