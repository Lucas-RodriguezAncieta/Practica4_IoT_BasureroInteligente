#include <Arduino.h>
#include <string.h>
#include "SmartBin.h"
#include "WiFiManager.h"
#include "AWSIoTManager.h"
#include "ShadowManager.h"
#include "CloudMessages.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

SmartBin bin;
static QueueHandle_t snapshots, commands, events;
static ShadowManager* shadowHandler=nullptr;
static void mqttCallback(char* topic,byte* payload,unsigned int length) {
  if (shadowHandler) shadowHandler->onMessage(topic,payload,length);
}
static BinSnapshot snapshot() {
  BinSnapshot s={};
  s.automatic=bin.mode()==SmartBin::Mode::Automatic;
  s.front=bin.proximityDetected(); s.obstacle=bin.obstacleDetected();
  s.measurementValid=bin.hasValidMeasurement(); s.servoReady=bin.servoReady();
  s.capacityLock=bin.capacityLock(); s.fill=bin.fillPercent(); s.distance=bin.distanceCm();
  s.autoCloseSeconds=bin.autoCloseSeconds(); s.threshold=bin.fillAlertThreshold();
  s.lid=static_cast<uint8_t>(bin.lidState());
  return s;
}
static void emitEvent(const char* type,const char* origin,const char* detail,const char* id="") {
  if (!events) return;
  BinEvent e={};
  strlcpy(e.type,type,sizeof(e.type)); strlcpy(e.origin,origin,sizeof(e.origin));
  strlcpy(e.detail,detail,sizeof(e.detail)); strlcpy(e.id,id,sizeof(e.id));
  if (xQueueSend(events,&e,0)!=pdTRUE) Serial.println("Cola de eventos llena; evento descartado.");
}
static void networkTask(void*) {
  WiFiManager wifi;
  AWSIoTManager aws;
  ShadowManager shadow(aws,commands);
  shadowHandler=&shadow;
  shadow.begin(); wifi.begin(); aws.begin(mqttCallback);
  bool wasConnected=false;
  BinSnapshot s={};
  for (;;) {
    wifi.update();
    bool connected=aws.update(wifi.ready());
    if (connected && !wasConnected) shadow.onConnected();
    if (!connected && wasConnected) { Serial.println("MQTT desconectado; reintentando."); shadow.onDisconnected(); }
    wasConnected=connected;
    xQueuePeek(snapshots,&s,0);
    if (connected) {
      shadow.update(s);
      BinEvent e;
      if (xQueueReceive(events,&e,0)==pdTRUE) shadow.event(e);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
void setup() {
  Serial.begin(115200);
  bin.begin();
  snapshots=xQueueCreate(1,sizeof(BinSnapshot));
  commands=xQueueCreate(8,sizeof(CloudCommand));
  events=xQueueCreate(16,sizeof(BinEvent));
  if (!snapshots || !commands || !events) { Serial.println("Error de memoria: red deshabilitada; control local activo."); return; }
  BinSnapshot s=snapshot(); xQueueOverwrite(snapshots,&s);
  if (xTaskCreatePinnedToCore(networkTask,"network",16384,nullptr,1,nullptr,0)!=pdPASS)
    Serial.println("Error creando tarea de red; control local activo.");
}
void loop() {
  static bool initialized=false;
  static BinSnapshot previous={};
  static char motionOrigin[6]="edge", motionId[65]="";
  static uint8_t motionTarget=0;
  static char modeOrigin[6]="edge", modeId[65]="";
  if (!initialized) { previous=snapshot(); initialized=true; }
  CloudCommand c;
  if (commands && xQueueReceive(commands,&c,0)==pdTRUE) {
    if (c.hasMode) {
      strlcpy(modeOrigin,"nube",sizeof(modeOrigin)); modeId[0]=0;
      bin.setMode(c.automatic ? SmartBin::Mode::Automatic : SmartBin::Mode::Manual);
    }
    if (c.hasHold) bin.setAutoCloseSeconds(c.hold);
    if (c.hasThreshold) bin.setFillAlertThreshold(c.threshold);
    if (c.hasLock) bin.setCapacityLock(c.lock);
    if (c.action[0]) {
      bool opening=!strcmp(c.action,"abrir");
      bool allowed=bin.servoReady() && (!opening || bin.openingAllowed());
      emitEvent("orden",c.origin,allowed ? "aceptada" : "rechazada",c.id);
      if (allowed) {
        LidServo::State before=bin.lidState();
        motionTarget=0;
        if ((opening && before!=LidServo::State::Open && before!=LidServo::State::Opening) ||
            (!opening && before!=LidServo::State::Closed && before!=LidServo::State::Closing)) {
          motionTarget=opening ? 1 : 3;
          strlcpy(motionOrigin,c.origin,sizeof(motionOrigin)); strlcpy(motionId,c.id,sizeof(motionId));
        }
        if (opening) bin.requestOpen(); else bin.requestClose();
        Serial.printf("Comando %s procesado: %s solicitado.\n",c.id,c.action);
      } else Serial.printf("Comando %s rechazado por bloqueo o servo no disponible.\n",c.id);
    }
  }
  bin.update();
  BinSnapshot current=snapshot();
  if (current.automatic!=previous.automatic) {
    emitEvent("cambio_modo",modeOrigin,current.automatic ? "AUTOMATICO" : "MANUAL",modeId);
  }
  strlcpy(modeOrigin,"edge",sizeof(modeOrigin)); modeId[0]=0;
  if (current.obstacle && !previous.obstacle) emitEvent("obstaculo","edge","detectado");
  if (current.lid!=previous.lid) {
    if (current.lid==1 || current.lid==3) {
      bool commanded=current.lid==motionTarget;
      emitEvent(current.lid==1 ? "apertura" : "cierre",commanded ? motionOrigin : "edge",
                "movimiento_estimado",commanded ? motionId : "");
      if (commanded) { motionTarget=0; strlcpy(motionOrigin,"edge",sizeof(motionOrigin)); motionId[0]=0; }
    }
  }
  previous=current;
  if (snapshots) xQueueOverwrite(snapshots,&current);
}
