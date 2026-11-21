#pragma once
#include <Arduino.h>
namespace BinConfig {
constexpr uint8_t IR1_PIN=27, IR2_PIN=26, TRIG_PIN=18, ECHO_PIN=19, SERVO_PIN=21;
// Cambiar a HIGH para módulos activos en alto.
constexpr uint8_t IR1_ACTIVE=LOW, IR2_ACTIVE=LOW;
constexpr unsigned long DEBOUNCE_MS=50, STEP_MS=20, HOLD_MS=5000;
constexpr int MIN_ANGLE=0, MAX_ANGLE=180, CLOSED_ANGLE=0, OPEN_ANGLE=90, STEP_DEGREES=1;
// PROVISIONAL: calibrar estas distancias en el recipiente real.
constexpr float EMPTY_CM=30.0f, FULL_CM=5.0f;
constexpr float MIN_CM=2.0f, MAX_CM=400.0f;
constexpr unsigned long ECHO_TIMEOUT_US=25000, SAMPLE_MS=70, MEASURE_MS=30000, STATUS_MS=10000;
constexpr uint8_t SAMPLE_COUNT=5, MIN_VALID=3;
}
