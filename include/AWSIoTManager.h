#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
class AWSIoTManager {
 public:
  using Handler=void (*)(char*,byte*,unsigned int);
  AWSIoTManager();
  void begin(Handler handler);
  bool update(bool networkReady);
  bool publish(const char* topic,const char* payload);
  bool subscribe(const char* topic);
  bool connected() { return mqtt_.connected(); }
 private:
  WiFiClientSecure tls_;
  PubSubClient mqtt_;
  unsigned long lastAttempt_=0;
};
