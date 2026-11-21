#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "AWSIoTManager.h"
#include "CloudMessages.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
class ShadowManager {
 public:
  ShadowManager(AWSIoTManager& aws, QueueHandle_t commands);
  void begin();
  void onConnected();
  void onDisconnected();
  void onMessage(const char* topic,const byte* payload,unsigned int length);
  void update(const BinSnapshot& snapshot);
  void event(const BinEvent& event);
 private:
  void applyDesired(JsonObjectConst desired, uint32_t version);
  void requestGet();
  void report(const BinSnapshot& snapshot);
  bool sendJson(const char* topic,JsonDocument& doc);
  AWSIoTManager& aws_;
  QueueHandle_t commands_;
  Preferences prefs_;
  bool preferencesReady_=false, subscribed_=false, synced_=false, getPending_=false, clearPending_=false;
  uint32_t lastId_=0;
  unsigned long lastGet_=0,lastReport_=0,lastTelemetry_=0,lastClear_=0;
  char base_[160]={},get_[168]={},update_[168]={},telemetry_[160]={},events_[160]={};
};
