#include "WiFiManager.h"
#include "CloudConfig.h"
#include <WiFi.h>
#include <time.h>
void WiFiManager::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  lastAttempt_=millis()-CloudConfig::WIFI_RETRY_MS;
  if (!CloudSecrets::WIFI_SSID[0]) Serial.println("WiFi sin configurar: complete CloudSecrets.h.");
}
bool WiFiManager::ready() const { return WiFi.status()==WL_CONNECTED && time(nullptr)>1700000000; }
void WiFiManager::update() {
  unsigned long now=millis();
  if (WiFi.status()!=WL_CONNECTED) {
    if (connected_) { Serial.println("WiFi desconectado."); connected_=false; }
    if (!CloudSecrets::WIFI_SSID[0]) return;
    if (now-lastAttempt_>=CloudConfig::WIFI_RETRY_MS) {
      lastAttempt_=now;
      Serial.println("Intentando conectar WiFi...");
      WiFi.disconnect();
      WiFi.begin(CloudSecrets::WIFI_SSID,CloudSecrets::WIFI_PASSWORD);
    }
    return;
  }
  if (!connected_) {
    connected_=true;
    Serial.print("WiFi conectado. IP: "); Serial.println(WiFi.localIP());
  }
  if (!clockStarted_) {
    configTime(0,0,"pool.ntp.org","time.nist.gov");
    clockStarted_=true;
    Serial.println("Esperando sincronizacion horaria NTP.");
  }
  if (!timeAnnounced_ && time(nullptr)>1700000000) {
    timeAnnounced_=true; Serial.println("Reloj sincronizado para TLS.");
  }
}
