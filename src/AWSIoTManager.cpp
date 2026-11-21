#include "AWSIoTManager.h"
#include "CloudConfig.h"
AWSIoTManager::AWSIoTManager():mqtt_(tls_) {}
void AWSIoTManager::begin(Handler handler) {
  if (!CloudSecrets::AWS_ENDPOINT[0] || !CloudSecrets::THING_NAME[0])
    Serial.println("AWS sin configurar: complete CloudSecrets.h.");
  tls_.setCACert(CloudSecrets::ROOT_CA);
  tls_.setCertificate(CloudSecrets::DEVICE_CERT);
  tls_.setPrivateKey(CloudSecrets::PRIVATE_KEY);
  tls_.setTimeout(2);
  mqtt_.setServer(CloudSecrets::AWS_ENDPOINT,CloudConfig::MQTT_PORT);
  mqtt_.setCallback(handler);
  mqtt_.setSocketTimeout(2);
  if (!mqtt_.setBufferSize(CloudConfig::MQTT_BUFFER_SIZE)) Serial.println("Error: no se pudo reservar el buffer MQTT.");
  lastAttempt_=millis()-CloudConfig::MQTT_RETRY_MS;
}
bool AWSIoTManager::update(bool networkReady) {
  if (!networkReady) {
    if (mqtt_.connected()) mqtt_.disconnect();
    return false;
  }
  if (mqtt_.connected()) { mqtt_.loop(); return true; }
  if (!CloudSecrets::AWS_ENDPOINT[0] || !CloudSecrets::THING_NAME[0]) return false;
  unsigned long now=millis();
  if (now-lastAttempt_<CloudConfig::MQTT_RETRY_MS) return false;
  lastAttempt_=now;
  Serial.println("Conectando MQTT TLS a AWS IoT...");
  if (mqtt_.connect(CloudSecrets::THING_NAME)) {
    Serial.println("MQTT conectado."); return true;
  }
  Serial.printf("Error MQTT: %d; se reintentara.\n",mqtt_.state());
  return false;
}
bool AWSIoTManager::publish(const char* topic,const char* payload) { return mqtt_.connected() && mqtt_.publish(topic,payload); }
bool AWSIoTManager::subscribe(const char* topic) {
  bool ok=mqtt_.subscribe(topic);
  Serial.printf("Suscripcion %s: %s\n",topic,ok ? "solicitada" : "error");
  return ok;
}
