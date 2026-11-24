# T13–T15 — Pruebas de AWS IoT y DynamoDB

Resultados comunicados por el compañero responsable de AWS. Las publicaciones se realizaron desde el cliente de pruebas MQTT sobre el Thing simulado `basurero-grupo3-02`; no representan mediciones físicas del ESP32. Las salidas de consola se transcriben según el registro aportado por el equipo. Las capturas o exportaciones correspondientes deben enlazarse cuando estén disponibles.

## T13 — Regla IoT y Lambda

Se publicó en `basurero/basurero-grupo3-02/telemetria` una telemetría con `distanceCm: 42.38`, `fillPercent: 83`, `mode: AUTOMATICO`, `lidState: cerrada` y `testId: T13-002`. Según el equipo, IoT Core recibió el mensaje, la regla invocó la Lambda, CloudWatch mostró `Registro guardado: basurero-grupo3-02 - TELEMETRIA` y se verificó el elemento correspondiente en `basurero_historial`.

Los valores son datos de prueba simulados; no representan una medición calibrada del recipiente.

**Resultado comunicado: APROBADO.**

## T14 — Almacenamiento de telemetría y eventos

El equipo informó que publicó y encontró en DynamoDB 10 telemetrías y cinco eventos, todos simulados desde MQTT Test Client. Las telemetrías utilizaron `mode: AUTOMATICO`, `lidState: cerrada`, `source: demo-t14` y `testId` de `T14-001` a `T14-010`.

| Distancia enviada (cm) | Llenado enviado (%) |
|---:|---:|
| 38,5 | 20 |
| 35,2 | 30 |
| 30,8 | 40 |
| 25,4 | 50 |
| 20,1 | 60 |
| 17,8 | 70 |
| 13,5 | 80 |
| 10,2 | 85 |
| 7,8 | 90 |
| 5,5 | 95 |

Estos pares son datos de prueba para almacenamiento y no deben interpretarse como calibración ni como pares calculados por la fórmula del firmware.

Se informaron cinco eventos publicados en `basurero/basurero-grupo3-02/eventos`:

| Evento | Origen | Detalle | Identificador |
|---|---|---|---|
| apertura | edge | movimiento_estimado | — |
| cierre | edge | movimiento_estimado | — |
| obstaculo | edge | detectado | — |
| cambio_modo | nube | MANUAL | — |
| orden | voz | aceptada | T14-E05 |

Los orígenes son **etiquetas de mensajes simulados** y no representan ejecuciones físicas ni comandos reales de Alexa. El equipo confirmó los 15 registros; no se dispone aquí de capturas individuales enlazadas. Los campos auxiliares enviados no necesariamente se conservan todos en el elemento DynamoDB.

**Resultado comunicado: APROBADO, 15/15 registros.**

## T15 — Validación de mensajes inválidos

El equipo informó que confirmó los siguientes descartes en CloudWatch:

| Caso | Entrada inválida | Mensaje comunicado |
|---|---|---|
| T15-001 | `distanceCm: 500` | `Mensaje descartado: distanceCm: fuera de rango` |
| T15-002 | Telemetría sin `mode` | `Mensaje descartado: mode: texto invalido` |
| T15-003 | `distanceCm: "treinta"` | `Mensaje descartado: distanceCm: tipo invalido` |
| T15-004 | `lidState: "bloqueada"` | `Mensaje descartado: lidState: valor no permitido` |
| T15-005 | Evento con `origin: app` | `Mensaje descartado: origin: valor no permitido` |

**Resultado comunicado: APROBADO, 5/5 rechazos observados en CloudWatch.** El registro proporcionado no incluye capturas de la consulta a DynamoDB para cada mensaje rechazado.
