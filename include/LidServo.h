#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
class LidServo {
 public:
  enum class State { Closed, Opening, Open, Closing };
  bool begin(bool startOpen);
  void update(unsigned long now);
  void open();
  void close();
  State state() const { return state_; }
  bool ready() const { return ready_; }
 private:
  Servo servo_;
  State state_=State::Closed;
  int angle_=0, closed_=0, open_=0;
  unsigned long lastStep_=0;
  bool ready_=false;
};
