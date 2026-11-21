#pragma once
#include <Arduino.h>
class ObstacleSensor {
 public:
  void begin();
  void update(unsigned long now);
  bool detected() const { return detected_; }
 private:
  bool detected_=false, raw_=false;
  unsigned long changed_=0;
};
