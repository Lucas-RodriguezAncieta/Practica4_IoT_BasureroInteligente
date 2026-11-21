#pragma once
#include <Arduino.h>
struct BinSnapshot {
  bool automatic, front, obstacle, measurementValid, servoReady, capacityLock;
  float distance, fill;
  unsigned int autoCloseSeconds, threshold;
  uint8_t lid;
};
struct CloudCommand {
  bool hasMode, automatic, hasHold, hasThreshold, hasLock, lock;
  unsigned int hold, threshold;
  char action[8], origin[6], id[65];
};
struct BinEvent {
  char type[24], origin[6], detail[32], id[65];
};
