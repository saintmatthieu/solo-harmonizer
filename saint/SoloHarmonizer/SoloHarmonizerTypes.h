#pragma once

namespace saint {
struct AudioConfig {
  int samplesPerSecond = 0;
  int samplesPerBlock = 0;
  int ticksPerCrotchet = 0;
  int crotchetsPerSecond = 0;
};
} // namespace saint
