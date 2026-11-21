#include "ObstacleSensor.h"
#include "BinConfig.h"
void ObstacleSensor::begin() {
  pinMode(BinConfig::IR2_PIN, INPUT);
  detected_=raw_=digitalRead(BinConfig::IR2_PIN)==BinConfig::IR2_ACTIVE;
  changed_=millis();
}
void ObstacleSensor::update(unsigned long now) {
  bool value=digitalRead(BinConfig::IR2_PIN)==BinConfig::IR2_ACTIVE;
  if (value!=raw_) { raw_=value; changed_=now; }
  // Activación inmediata; liberación filtrada.
  if (raw_) detected_=true;
  else if (now-changed_>=BinConfig::DEBOUNCE_MS) detected_=false;
}
