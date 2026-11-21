#pragma once
#if __has_include("CloudSecrets.h")
#include "CloudSecrets.h"
#else
#include "CloudSecrets.example.h"
#endif
namespace CloudConfig {
constexpr unsigned long WIFI_RETRY_MS=15000, MQTT_RETRY_MS=10000, TELEMETRY_MS=30000;
constexpr unsigned long SHADOW_RETRY_MS=10000, REPORTED_MS=30000;
constexpr unsigned int MQTT_BUFFER_SIZE=8192;
constexpr unsigned int MQTT_PORT=8883;
}
