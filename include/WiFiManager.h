#pragma once
#include <Arduino.h>
class WiFiManager {
 public:
  void begin();
  void update();
  bool ready() const;
 private:
  unsigned long lastAttempt_=0;
  bool connected_=false, clockStarted_=false, timeAnnounced_=false;
};
