#include "ShadowManager.h"
#include "CloudConfig.h"
#include <time.h>
#include <string.h>
static const char* lidName(uint8_t state) {
  switch (state) { case 0:return "cerrada"; case 1:return "abriendo"; case 2:return "abierta"; case 3:return "cerrando"; default:return "desconocida"; }
}
ShadowManager::ShadowManager(AWSIoTManager& aws,QueueHandle_t commands):aws_(aws),commands_(commands) {}
void ShadowManager::begin() {
  snprintf(base_,sizeof(base_),"$aws/things/%s/shadow",CloudSecrets::THING_NAME);
  snprintf(get_,sizeof(get_),"%s/get",base_);
  snprintf(update_,sizeof(update_),"%s/update",base_);
  snprintf(telemetry_,sizeof(telemetry_),"basurero/%s/telemetria",CloudSecrets::THING_NAME);
  snprintf(events_,sizeof(events_),"basurero/%s/eventos",CloudSecrets::THING_NAME);
  preferencesReady_=prefs_.begin("bin-shadow",false);
  if (preferencesReady_) lastId_=prefs_.getUInt("lastId",0);
  else Serial.println("Error NVS: comandos puntuales deshabilitados.");
}
void ShadowManager::onConnected() {
  const char* suffixes[]={"/get/accepted","/get/rejected","/update/delta","/update/accepted","/update/rejected"};
  subscribed_=true;
  for (const char* suffix:suffixes) {
    char topic[200]; snprintf(topic,sizeof(topic),"%s%s",base_,suffix);
    if (!aws_.subscribe(topic)) subscribed_=false;
  }
  synced_=false; getPending_=false; clearPending_=false;
  if (subscribed_) requestGet();
  else Serial.println("Error enviando suscripciones; se reintentara.");
}
void ShadowManager::onDisconnected() { subscribed_=synced_=getPending_=clearPending_=false; }
void ShadowManager::requestGet() {
  if (aws_.publish(get_,"{}")) {
    lastGet_=millis(); getPending_=true;
    Serial.println("Shadow: solicitud get enviada.");
  }
}
bool ShadowManager::sendJson(const char* topic,JsonDocument& doc) {
  char output[2048];
  if (doc.overflowed() || measureJson(doc)>=sizeof(output)) { Serial.println("Error: JSON excede el buffer de salida."); return false; }
  serializeJson(doc,output,sizeof(output));
  return aws_.publish(topic,output);
}
void ShadowManager::applyDesired(JsonObjectConst desired,uint32_t version) {
  CloudCommand c={};
  if (desired["mode"].is<const char*>()) {
    const char* m=desired["mode"];
    if (!strcmp(m,"AUTOMATICO") || !strcmp(m,"MANUAL")) { c.hasMode=true; c.automatic=!strcmp(m,"AUTOMATICO"); }
    else Serial.println("Shadow: mode invalido.");
  }
  if (desired["autoCloseSeconds"].is<unsigned int>()) {
    unsigned int v=desired["autoCloseSeconds"];
    if (v>=1 && v<=3600) { c.hasHold=true; c.hold=v; } else Serial.println("Shadow: autoCloseSeconds fuera de rango.");
  }
  if (desired["fillAlertThreshold"].is<unsigned int>()) {
    unsigned int v=desired["fillAlertThreshold"];
    if (v>=1 && v<=100) { c.hasThreshold=true; c.threshold=v; } else Serial.println("Shadow: fillAlertThreshold fuera de rango.");
  }
  if (desired["capacityLock"].is<bool>()) { c.hasLock=true; c.lock=desired["capacityLock"]; }
  bool hasSettings=c.hasMode || c.hasHold || c.hasThreshold || c.hasLock;
  JsonObjectConst cmd=desired["command"].as<JsonObjectConst>();
  bool clear=false;
  if (!cmd.isNull()) {
    uint32_t id=cmd["id"] | 0U;
    const char* action=cmd["action"] | "";
    const char* origin=cmd["origin"] | "nube";
    bool valid=id && (!strcmp(action,"abrir") || !strcmp(action,"cerrar")) &&
               (!strcmp(origin,"nube") || !strcmp(origin,"voz"));
    if (!valid) Serial.println("Shadow: comando invalido (id, action u origin).");
    else if (id<=lastId_) { Serial.printf("Comando %lu ya procesado o antiguo.\n",static_cast<unsigned long>(id)); clear=true; }
    else if (!preferencesReady_) Serial.println("Shadow: comando no admitido; NVS no disponible.");
    else if (uxQueueSpacesAvailable(commands_)==0) Serial.println("Shadow: cola de comandos llena; se reintentara.");
    else if (prefs_.putUInt("lastId",id)!=sizeof(uint32_t)) Serial.println("Shadow: no se pudo guardar id; comando no ejecutado.");
    else {
      lastId_=id;
      strlcpy(c.action,action,sizeof(c.action)); strlcpy(c.origin,origin,sizeof(c.origin));
      snprintf(c.id,sizeof(c.id),"%lu",static_cast<unsigned long>(id));
      clear=true;
      Serial.printf("Comando %s registrado para procesamiento local.\n",c.id);
    }
  } else if (desired.containsKey("command")) Serial.println("Shadow: command debe ser un objeto.");
  if (hasSettings || c.action[0]) {
    if (xQueueSend(commands_,&c,0)!=pdTRUE) {
      Serial.println("Shadow: cola llena; ajustes pendientes de reconciliacion.");
      if (c.action[0]) Serial.println("Advertencia: id registrado pero orden no encolada.");
      clear=false;
    }
  }
  if (clear && version) {
    StaticJsonDocument<256> doc;
    doc["state"]["desired"]["command"]=nullptr;
    doc["version"]=version;
    doc["clientToken"]="clear-command";
    if (sendJson(update_,doc)) { clearPending_=true; lastClear_=millis(); Serial.println("Shadow: solicitada limpieza condicional del comando."); }
  }
}
void ShadowManager::onMessage(const char* topic,const byte* payload,unsigned int length) {
  StaticJsonDocument<6144> doc;
  DeserializationError err=deserializeJson(doc,payload,length);
  if (err) { Serial.printf("Shadow JSON invalido o demasiado grande: %s\n",err.c_str()); return; }
  const char* suffix=topic+strlen(base_);
  if (!strcmp(suffix,"/get/accepted")) {
    Serial.println("Shadow recibido."); getPending_=false; synced_=true;
    applyDesired(doc["state"]["desired"].as<JsonObjectConst>(),doc["version"] | 0U);
    lastReport_=millis()-CloudConfig::REPORTED_MS;
  } else if (!strcmp(suffix,"/get/rejected")) {
    getPending_=false;
    int code=doc["code"] | 0;
    Serial.printf("Shadow get rechazado: %d.\n",code);
    if (code==404) { synced_=true; lastReport_=millis()-CloudConfig::REPORTED_MS; }
    else { synced_=false; lastGet_=millis(); }
  } else if (!strcmp(suffix,"/update/delta")) {
    Serial.println("Shadow delta recibido.");
    applyDesired(doc["state"].as<JsonObjectConst>(),doc["version"] | 0U);
  } else if (!strcmp(suffix,"/update/accepted")) {
    const char* token=doc["clientToken"] | "";
    if (!strcmp(token,"clear-command")) { clearPending_=false; Serial.println("Shadow: limpieza de comando confirmada."); }
    else if (!strcmp(token,"reported")) Serial.println("Shadow: reported aceptado.");
  } else if (!strcmp(suffix,"/update/rejected")) {
    int code=doc["code"] | 0;
    Serial.printf("Shadow update rechazado: %d.\n",code);
    clearPending_=false; synced_=false; getPending_=false; lastGet_=millis()-CloudConfig::SHADOW_RETRY_MS;
  }
}
void ShadowManager::report(const BinSnapshot& s) {
  StaticJsonDocument<768> doc;
  JsonObject r=doc.createNestedObject("state").createNestedObject("reported");
  r["mode"]=s.automatic ? "AUTOMATICO" : "MANUAL";
  r["lidState"]=s.servoReady ? lidName(s.lid) : "no_disponible";
  if (s.measurementValid) { r["fillPercent"]=s.fill; r["distanceCm"]=s.distance; }
  else { r["fillPercent"]=nullptr; r["distanceCm"]=nullptr; }
  r["irFrontDetected"]=s.front; r["irObstacleDetected"]=s.obstacle;
  r["autoCloseSeconds"]=s.autoCloseSeconds; r["fillAlertThreshold"]=s.threshold; r["capacityLock"]=s.capacityLock;
  doc["clientToken"]="reported";
  if (sendJson(update_,doc)) Serial.println("Shadow: reported enviado.");
  else Serial.println("Shadow: no se pudo enviar reported.");
}
void ShadowManager::update(const BinSnapshot& s) {
  if (!aws_.connected()) return;
  unsigned long now=millis();
  if (!subscribed_) { if (now-lastGet_>=CloudConfig::SHADOW_RETRY_MS) { lastGet_=now; onConnected(); } return; }
  if (!synced_ && now-lastGet_>=CloudConfig::SHADOW_RETRY_MS) requestGet();
  if (clearPending_ && now-lastClear_>=CloudConfig::SHADOW_RETRY_MS) { clearPending_=false; synced_=false; requestGet(); }
  if (synced_ && !clearPending_ && now-lastReport_>=CloudConfig::REPORTED_MS) { lastReport_=now; report(s); }
  if (now-lastTelemetry_>=CloudConfig::TELEMETRY_MS) {
    lastTelemetry_=now;
    StaticJsonDocument<384> doc;
    if (s.measurementValid) { doc["fillPercent"]=s.fill; doc["distanceCm"]=s.distance; }
    else { doc["fillPercent"]=nullptr; doc["distanceCm"]=nullptr; }
    doc["lidState"]=s.servoReady ? lidName(s.lid) : "no_disponible";
    doc["mode"]=s.automatic ? "AUTOMATICO" : "MANUAL";
    time_t timestamp=time(nullptr); if (timestamp>1700000000) doc["timestamp"]=static_cast<uint32_t>(timestamp);
    if (!sendJson(telemetry_,doc)) Serial.println("Error publicando telemetria.");
  }
}
void ShadowManager::event(const BinEvent& e) {
  StaticJsonDocument<384> doc;
  doc["event"]=e.type; doc["origin"]=e.origin; doc["detail"]=e.detail;
  if (e.id[0]) doc["commandId"]=e.id;
  time_t timestamp=time(nullptr); if (timestamp>1700000000) doc["timestamp"]=static_cast<uint32_t>(timestamp);
  if (!sendJson(events_,doc)) Serial.println("Evento no publicado: conexion no disponible.");
}
