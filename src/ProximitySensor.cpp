#include "ProximitySensor.h"
#include "BinConfig.h"
void ProximitySensor::begin() {
  pinMode(BinConfig::IR1_PIN, INPUT);
  stable_=raw_=digitalRead(BinConfig::IR1_PIN)==BinConfig::IR1_ACTIVE;
  changed_=millis();
}
void ProximitySensor::update(unsigned long now) {
  bool value=digitalRead(BinConfig::IR1_PIN)==BinConfig::IR1_ACTIVE;
  if (value!=raw_) { raw_=value; changed_=now; }
  if (now-changed_>=BinConfig::DEBOUNCE_MS) stable_=raw_;
}
