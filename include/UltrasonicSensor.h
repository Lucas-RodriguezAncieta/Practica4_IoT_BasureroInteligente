#pragma once
#include <Arduino.h>
#include "BinConfig.h"
class UltrasonicSensor {
 public:
  enum class Result { None, Valid, Invalid };
  void begin();
  void start(unsigned long now);
  void cancel() { busy_=false; }
  Result update(unsigned long now);
  bool busy() const { return busy_; }
  float distanceCm() const { return distance_; }
  float fillPercent() const { return fill_; }
 private:
  float samples_[BinConfig::SAMPLE_COUNT]={};
  uint8_t attempts_=0, valid_=0;
  bool busy_=false;
  unsigned long lastSample_=0;
  float distance_=0, fill_=0;
};
