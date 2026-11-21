#pragma once
#include <Arduino.h>
#include "ProximitySensor.h"
#include "ObstacleSensor.h"
#include "UltrasonicSensor.h"
#include "LidServo.h"
#include "BinConfig.h"
// Interfaz para futuros adaptadores de telemetría y comandos.
class SmartBin {
 public:
  enum class Mode { Automatic, Manual };
  void begin();
  void update();
  void setMode(Mode mode);
  void requestOpen();
  void requestClose();
  bool setAutoCloseSeconds(unsigned int seconds);
  bool setFillAlertThreshold(unsigned int percent);
  void setCapacityLock(bool enabled) { capacityLock_=enabled; }
  unsigned int autoCloseSeconds() const { return autoCloseSeconds_; }
  unsigned int fillAlertThreshold() const { return fillAlertThreshold_; }
  bool capacityLock() const { return capacityLock_; }
  bool servoReady() const { return lid_.ready(); }
  bool openingAllowed() const { return !capacityLock_ || (validMeasurement_ && fillPercent()<fillAlertThreshold_); }
  Mode mode() const { return mode_; }
  LidServo::State lidState() const { return lid_.state(); }
  bool proximityDetected() const { return proximity_.detected(); }
  bool obstacleDetected() const { return obstacle_.detected(); }
  bool hasValidMeasurement() const { return validMeasurement_; }
  float distanceCm() const { return ultrasonic_.distanceCm(); }
  float fillPercent() const { return ultrasonic_.fillPercent(); }
 private:
  void readSerial();
  void command(const char* value);
  void printStatus() const;
  ProximitySensor proximity_;
  ObstacleSensor obstacle_;
  UltrasonicSensor ultrasonic_;
  LidServo lid_;
  Mode mode_=Mode::Automatic;
  LidServo::State lastState_=LidServo::State::Closed;
  bool lastProximity_=false, lastObstacle_=false, validMeasurement_=false, closeRequested_=false;
  unsigned long openedAt_=0, lastMeasurement_=0, lastStatus_=0;
  unsigned int autoCloseSeconds_=BinConfig::HOLD_MS/1000;
  unsigned int fillAlertThreshold_=90;
  bool capacityLock_=false;
  char buffer_[24]={};
  uint8_t length_=0;
  bool overflow_=false;
};
