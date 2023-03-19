#include "DisplayComponentHelper.h"
namespace saint {
void setToImageWidth(const std::vector<IntervalSpan> &spans,
                     std::vector<IntervalSpan>::const_iterator &first,
                     std::vector<IntervalSpan>::const_iterator &last,
                     float crotchet) {
  const auto centralCrotchet = crotchet;
  const auto crotchetLeft = centralCrotchet - widthInCrotchets / 2.f;
  const auto crotchetRight = centralCrotchet + widthInCrotchets / 2.f;
  first = std::find_if(first, spans.end(),
                       [crotchetLeft](const IntervalSpan &span) {
                         return span.beginCrotchet >= crotchetLeft;
                       });
  if (first != spans.begin()) {
    --first;
  }
  last = std::find_if(first, spans.end(),
                      [crotchetRight](const IntervalSpan &span) {
                        return span.beginCrotchet > crotchetRight;
                      });
}
} // namespace saint