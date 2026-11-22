
# AWS IoT Core y Device Shadow

[Volver al índice](README.md)

## Configuración

**Región:** `us-east-2`

| Recurso | Nombre |
|---|---|
| Thing físico | `basurero-grupo3-01` |
| Thing simulado | `basurero-grupo3-02` |
| Política IoT física | `BasureroP4IoTPolicy` |
| Tipo de Shadow | Clásico, sin nombre |

El primer Thing utiliza un certificado X.509 y una clave privada para conectarse por MQTT sobre TLS.

El segundo Thing fue creado sin certificado propio porque se utilizó únicamente para pruebas simuladas desde AWS.

## Autenticación y autorización

La conexión física utiliza:
- Endpoint AWS IoT Data-ATS.
- Certificado X.509.
- Clave privada del dispositivo.
- Política de AWS IoT Core.
- MQTT sobre TLS.

La política IoT autoriza operaciones como `iot:Connect`, `iot:Publish`, `iot:Subscribe` e `iot:Receive`, restringidas según los recursos configurados.

El Client ID autorizado del ESP32 es `basurero-grupo3-01`.

La política de ejemplo existente en la raíz del repositorio es:

[`aws-iot-policy.example.json`](../../aws-iot-policy.example.json)

No se publican certificados privados ni credenciales.

## ¿Qué es un Device Shadow?

Es un documento JSON que conserva el estado deseado y reportado de un Thing.

| Sección | Función |
|---|---|
| `desired` | Configuración u orden solicitada |
| `reported` | Último estado informado por el dispositivo |
| `delta` | Diferencia pendiente entre ambos |

### Atributos principales

| Atributo | Tipo | Descripción |
|---|---|---|
| `mode` | String | `MANUAL` o `AUTOMATICO` |
| `fillAlertThreshold` | Number | Umbral de bloqueo |
| `autoCloseSeconds` | Number | Tiempo de cierre |
| `capacityLock` | Boolean | Bloqueo por capacidad |
| `fillPercent` | Number | Llenado reportado |
| `distanceCm` | Number | Distancia medida |
| `lidState` | String | Estado de la tapa |
| `irFrontDetected` | Boolean | Detección frontal |
| `irObstacleDetected` | Boolean | Obstáculo de seguridad |
| `command` | Object | Orden de apertura o cierre |

Los atributos de sensores y estado físico se consultan principalmente desde `reported`.

## Tópicos de Shadow

Para un Thing con nombre `{thingName}`:

```text
$aws/things/{thingName}/shadow/get
$aws/things/{thingName}/shadow/get/accepted
$aws/things/{thingName}/shadow/update
$aws/things/{thingName}/shadow/update/accepted
$aws/things/{thingName}/shadow/update/delta
```

## Ejemplo de sincronización

1. Alexa solicita cambiar a modo manual.
2. `basurero_alexa` actualiza `desired.mode`.
3. AWS genera una diferencia `delta`, si corresponde.
4. El ESP32 procesa el cambio.
5. El ESP32 actualiza `reported.mode`.

La confirmación de Alexa no garantiza por sí sola que el actuador ya terminó la acción.

## Thing simulado

El Thing 02 conserva su propio Shadow.

En una prueba se configuró `reported.fillPercent = 35`, sin ningún ESP32 conectado.

Una modificación de `desired` puede dejar un `delta` pendiente porque no existe un dispositivo físico que actualice `reported`.

## Archivos relacionados

- `config/shadows/thing-01.example.json`
- `config/shadows/thing-02.example.json`
- [Permisos IAM](08-permisos-iam.md)
