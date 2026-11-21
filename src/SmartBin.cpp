#include "SmartBin.h"
#include "BinConfig.h"
#include <string.h>
static const char* stateName(LidServo::State state) {
  switch (state) {
    case LidServo::State::Closed: return "cerrada";
    case LidServo::State::Opening: return "abriendo";
    case LidServo::State::Open: return "abierta";
    case LidServo::State::Closing: return "cerrando";
  }
  return "desconocida";
}
void SmartBin::begin() {
  proximity_.begin(); obstacle_.begin(); ultrasonic_.begin();
  if (!lid_.begin(obstacle_.detected()))
    Serial.println("Servo no disponible; medicion de llenado suspendida porque no se puede estimar la tapa cerrada.");
  lastProximity_=proximity_.detected();
  lastObstacle_=obstacle_.detected();
  unsigned long now=millis();
  lastState_=lid_.state();
  if (lastState_==LidServo::State::Open) openedAt_=now;
  lastMeasurement_=now-BinConfig::MEASURE_MS; lastStatus_=now;
  Serial.println("Comandos: auto, manual, abrir, cerrar, estado.");
  Serial.println("Posicion de tapa estimada: no hay realimentacion del servo.");
  printStatus();
}
void SmartBin::setMode(Mode mode) {
  mode_=mode;
  Serial.printf("Modo: %s\n",mode_==Mode::Automatic ? "AUTOMATICO" : "MANUAL");
}
bool SmartBin::setAutoCloseSeconds(unsigned int seconds) {
  if (seconds<1 || seconds>3600) return false;
  autoCloseSeconds_=seconds; return true;
}
bool SmartBin::setFillAlertThreshold(unsigned int percent) {
  if (percent<1 || percent>100) return false;
  fillAlertThreshold_=percent; return true;
}
void SmartBin::requestOpen() {
  if (!lid_.ready()) { Serial.println("Error: servo no disponible."); return; }
  if (!openingAllowed()) { Serial.println("Apertura rechazada: bloqueo de capacidad activo."); return; }
  closeRequested_=false;
  ultrasonic_.cancel();
  lid_.open();
}
void SmartBin::requestClose() {
  if (!lid_.ready()) { Serial.println("Error: servo no disponible."); return; }
  if (lid_.state()==LidServo::State::Closed) return;
  closeRequested_=true;
  if (obstacle_.detected()) Serial.println("Cierre pendiente: obstaculo detectado.");
}
void SmartBin::update() {
  unsigned long now=millis();
  proximity_.update(now); obstacle_.update(now);
  if (proximity_.detected()!=lastProximity_) {
    lastProximity_=proximity_.detected();
    Serial.printf("IR1: %s\n",lastProximity_ ? "detectado" : "libre");
    if (lastProximity_ && mode_==Mode::Automatic) requestOpen();
  }
  if (obstacle_.detected()!=lastObstacle_) {
    lastObstacle_=obstacle_.detected();
    Serial.printf("IR2: %s\n",lastObstacle_ ? "obstaculo" : "libre");
  }
  readSerial();
  if (lid_.state()==LidServo::State::Closing && obstacle_.detected()) {
    Serial.println("Obstaculo durante cierre: reabriendo.");
    closeRequested_=true; lid_.open();
  }
  if (lid_.state()==LidServo::State::Open && !obstacle_.detected() &&
      (closeRequested_ || (mode_==Mode::Automatic && now-openedAt_>=static_cast<unsigned long>(autoCloseSeconds_)*1000UL))) {
    closeRequested_=false; lid_.close();
  }
  lid_.update(now);
  if (lid_.state()!=lastState_) {
    lastState_=lid_.state();
    if (lastState_==LidServo::State::Open) openedAt_=now;
    Serial.printf("Tapa (estimada): %s\n",stateName(lastState_));
  }
  if (lid_.ready() && lid_.state()==LidServo::State::Closed) {
    if (!ultrasonic_.busy() && now-lastMeasurement_>=BinConfig::MEASURE_MS) {
      lastMeasurement_=now; ultrasonic_.start(now);
    }
    UltrasonicSensor::Result result=ultrasonic_.update(now);
    if (result==UltrasonicSensor::Result::Valid) {
      validMeasurement_=true;
      Serial.printf("Distancia: %.1f cm; llenado estimado: %.1f %%\n",distanceCm(),fillPercent());
    } else if (result==UltrasonicSensor::Result::Invalid) {
      validMeasurement_=false;
      Serial.println("Advertencia: medicion de llenado descartada; consulte el diagnostico anterior.");
    }
  } else if (ultrasonic_.busy()) ultrasonic_.cancel();
  if (now-lastStatus_>=BinConfig::STATUS_MS) { lastStatus_=now; printStatus(); }
}
void SmartBin::printStatus() const {
  Serial.printf("Modo: %s | IR1: %s | IR2: %s | Tapa estimada: %s%s",
    mode_==Mode::Automatic ? "AUTOMATICO" : "MANUAL",
    proximity_.detected() ? "detectado" : "libre", obstacle_.detected() ? "obstaculo" : "libre",
    stateName(lid_.state()),lid_.ready() ? "" : " (servo no disponible)");
  if (validMeasurement_) Serial.printf(" | Distancia: %.1f cm | Llenado: %.1f %%\n",distanceCm(),fillPercent());
  else Serial.println(" | Llenado: sin medicion valida");
}
void SmartBin::readSerial() {
  while (Serial.available()) {
    char c=static_cast<char>(Serial.read());
    if (c=='\r') continue;
    if (c=='\n') {
      if (overflow_) Serial.println("Comando demasiado largo.");
      else if (length_) { buffer_[length_]='\0'; command(buffer_); }
      length_=0; overflow_=false;
    } else if (!overflow_) {
      if (length_<sizeof(buffer_)-1) buffer_[length_++]=c;
      else overflow_=true;
    }
  }
}
void SmartBin::command(const char* value) {
  if (strcmp(value,"auto")==0) setMode(Mode::Automatic);
  else if (strcmp(value,"manual")==0) setMode(Mode::Manual);
  else if (strcmp(value,"abrir")==0) requestOpen();
  else if (strcmp(value,"cerrar")==0) requestClose();
  else if (strcmp(value,"estado")==0) printStatus();
  else Serial.println("Comando desconocido. Use: auto, manual, abrir, cerrar, estado.");
}
