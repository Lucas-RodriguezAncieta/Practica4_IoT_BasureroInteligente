#pragma once
#include <Arduino.h>
class ProximitySensor {
 public:
  void begin();
  void update(unsigned long now);
  bool detected() const { return stable_; }
 private:
  bool stable_=false, raw_=false;
  unsigned long changed_=0;
};
