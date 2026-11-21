#include "LidServo.h"
#include "BinConfig.h"
bool LidServo::begin(bool startOpen) {
  closed_=constrain(BinConfig::CLOSED_ANGLE, BinConfig::MIN_ANGLE, BinConfig::MAX_ANGLE);
  open_=constrain(BinConfig::OPEN_ANGLE, BinConfig::MIN_ANGLE, BinConfig::MAX_ANGLE);
  if (BinConfig::MIN_ANGLE < 0 || BinConfig::MAX_ANGLE > 180 ||
      BinConfig::MIN_ANGLE >= BinConfig::MAX_ANGLE ||
      BinConfig::CLOSED_ANGLE < BinConfig::MIN_ANGLE ||
      BinConfig::CLOSED_ANGLE > BinConfig::MAX_ANGLE ||
      BinConfig::OPEN_ANGLE < BinConfig::MIN_ANGLE ||
      BinConfig::OPEN_ANGLE > BinConfig::MAX_ANGLE || closed_==open_) {
    Serial.println("Error servo: angulos invalidos o de apertura y cierre iguales.");
    return false;
  }
  servo_.setPeriodHertz(50);
  // El valor devuelto puede identificar el canal PWM 0; no es una prueba de fallo.
  servo_.attach(BinConfig::SERVO_PIN, 500, 2400);
  if (!servo_.attached()) {
    Serial.printf("Error servo: no se pudo asociar al GPIO %u.\n", BinConfig::SERVO_PIN);
    return false;
  }
  Serial.printf("Servo asociado al GPIO %u; cierre %d grados, apertura %d grados.\n",
                BinConfig::SERVO_PIN, closed_, open_);
  angle_=startOpen ? open_ : closed_;
  state_=startOpen ? State::Open : State::Closed;
  servo_.write(angle_);
  ready_=true;
  return true;
}
void LidServo::open() {
  if (ready_) state_=angle_==open_ ? State::Open : State::Opening;
}
void LidServo::close() {
  if (ready_) state_=angle_==closed_ ? State::Closed : State::Closing;
}
void LidServo::update(unsigned long now) {
  if (!ready_ || (state_!=State::Opening && state_!=State::Closing) || now-lastStep_<BinConfig::STEP_MS) return;
  lastStep_=now;
  int target=state_==State::Opening ? open_ : closed_;
  if (angle_<target) angle_=min(angle_+BinConfig::STEP_DEGREES,target);
  else angle_=max(angle_-BinConfig::STEP_DEGREES,target);
  servo_.write(angle_);
  if (angle_==target) state_=target==open_ ? State::Open : State::Closed;
}
