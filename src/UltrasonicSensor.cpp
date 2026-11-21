#include "UltrasonicSensor.h"
void UltrasonicSensor::begin() {
  pinMode(BinConfig::TRIG_PIN,OUTPUT);
  pinMode(BinConfig::ECHO_PIN,INPUT);
  digitalWrite(BinConfig::TRIG_PIN,LOW);
}
void UltrasonicSensor::start(unsigned long now) {
  attempts_=valid_=0; busy_=true; lastSample_=now-BinConfig::SAMPLE_MS;
  if (BinConfig::EMPTY_CM<=BinConfig::FULL_CM)
    Serial.printf("Error de calibracion: vacio %.1f cm debe ser mayor que lleno %.1f cm.\n",
                  BinConfig::EMPTY_CM, BinConfig::FULL_CM);
}
UltrasonicSensor::Result UltrasonicSensor::update(unsigned long now) {
  if (!busy_ || now-lastSample_<BinConfig::SAMPLE_MS) return Result::None;
  lastSample_=now;
  bool echoAlreadyHigh=digitalRead(BinConfig::ECHO_PIN)==HIGH;
  if (echoAlreadyHigh)
    Serial.printf("Advertencia ultrasonico: ECHO GPIO %u en HIGH antes del disparo.\n", BinConfig::ECHO_PIN);
  digitalWrite(BinConfig::TRIG_PIN,LOW); delayMicroseconds(2);
  digitalWrite(BinConfig::TRIG_PIN,HIGH); delayMicroseconds(10);
  digitalWrite(BinConfig::TRIG_PIN,LOW);
  // Bloqueo acotado por ECHO_TIMEOUT_US para cada muestra.
  unsigned long duration=pulseIn(BinConfig::ECHO_PIN,HIGH,BinConfig::ECHO_TIMEOUT_US);
  float cm=duration*0.0343f/2.0f;
  ++attempts_;
  if (echoAlreadyHigh) {
    Serial.printf("Muestra %u descartada: ECHO ya estaba en HIGH antes del disparo.\n", attempts_);
  } else if (!duration) {
    Serial.printf("Muestra %u sin eco medible: tiempo limite %lu us.\n",
                  attempts_, BinConfig::ECHO_TIMEOUT_US);
  } else if (cm<BinConfig::MIN_CM || cm>BinConfig::MAX_CM) {
    Serial.printf("Muestra %u fuera de rango: %lu us, %.1f cm (rango %.1f-%.1f cm).\n",
                  attempts_, duration, cm, BinConfig::MIN_CM, BinConfig::MAX_CM);
  } else {
    samples_[valid_++]=cm;
    Serial.printf("Muestra %u valida: %lu us, %.1f cm.\n", attempts_, duration, cm);
  }
  if (attempts_<BinConfig::SAMPLE_COUNT) return Result::None;
  busy_=false;
  if (valid_<BinConfig::MIN_VALID) {
    Serial.printf("Muestras insuficientes: %u de %u validas; se requieren %u.\n",
                  valid_, BinConfig::SAMPLE_COUNT, BinConfig::MIN_VALID);
  }
  if (valid_<BinConfig::MIN_VALID || BinConfig::EMPTY_CM<=BinConfig::FULL_CM) return Result::Invalid;
  for (uint8_t i=1;i<valid_;++i) {
    float value=samples_[i]; int j=i-1;
    while (j>=0 && samples_[j]>value) { samples_[j+1]=samples_[j]; --j; }
    samples_[j+1]=value;
  }
  distance_=valid_%2 ? samples_[valid_/2] : (samples_[valid_/2-1]+samples_[valid_/2])/2.0f;
  fill_=constrain(100.0f*(BinConfig::EMPTY_CM-distance_)/(BinConfig::EMPTY_CM-BinConfig::FULL_CM),0.0f,100.0f);
  return Result::Valid;
}
