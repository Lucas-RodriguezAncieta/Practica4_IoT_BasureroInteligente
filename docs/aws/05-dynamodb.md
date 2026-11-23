
# Amazon DynamoDB — Modelo de datos

[Volver al índice](README.md)

## Objetivo

Conservar mediciones y eventos de los basureros, así como las vinculaciones entre usuarios de Alexa y Things de AWS IoT Core.

Se configuraron dos tablas en `us-east-2`.

## Tablas implementadas

| Tabla | Partition Key | Sort Key |
|---|---|---|
| `basurero_historial` | `thingName` (String) | `timestampId` (String) |
| `basurero_usuarios` | `userId` (String) | No tiene |

## 1. basurero_historial

Esta tabla almacena telemetrías y eventos recibidos mediante `basurero_almacenamiento`.

### Atributos

| Atributo | Tipo | Función |
|---|---|---|
| `thingName` | String | Identificador del dispositivo |
| `timestampId` | String | Clave temporal del registro |
| `recordType` | String | `TELEMETRIA` o `EVENTO` |
| `receivedAt` | Number | Marca temporal de la regla |
| `fillPercent` | Number | Porcentaje de llenado |
| `distanceCm` | Number | Distancia ultrasónica |
| `mode` | String | Modo de funcionamiento |
| `lidState` | String | Estado de la tapa |
| `irFrontDetected` | Boolean | Sensor frontal |
| `irObstacleDetected` | Boolean | Sensor de obstáculos |
| `origin` | String | Procedencia del evento |
| `commandId` | Number/String | Identificador de comando |

Los campos no pertenecientes a la clave pueden ser opcionales, según el tipo de mensaje.

### Procedencia de eventos

| Valor | Origen |
|---|---|
| `edge` | Dispositivo físico |
| `voz` | Solicitud de Alexa |
| `nube` | Procesamiento en AWS |

La procedencia debe corresponder al evento real. No se genera automáticamente por modificar el Shadow.

### Patrón de consulta

Buscar todos los registros de un Thing:

```text
thingName = basurero-grupo3-01
```

La clave `timestampId` permite ordenar los registros dentro de la partición según su formato.

## 2. basurero_usuarios

Esta tabla almacena la relación entre usuarios y dispositivos.

### Atributos

| Atributo | Tipo | Función |
|---|---|---|
| `userId` | String | Clave primaria |
| `thingName` | String | Dispositivo vinculado |
| `commandSeq` | Number | Contador de comandos |

### Vinculación

Ejemplo:

```json
{
  "userId": "usuario-prueba-002",
  "thingName": "basurero-grupo3-02"
}
```

La Lambda `basurero_alexa` utiliza `GetItem` para resolver el Thing vinculado.

### Contadores

Se utilizan registros especiales como:

```text
#seq#basurero-grupo3-01
```

El atributo `commandSeq` se incrementa mediante `UpdateItem`.

Estos elementos no representan usuarios reales; permiten generar identificadores persistentes de comandos.

## Operaciones IAM

| Lambda | Tabla | Operaciones |
|---|---|---|
| `basurero_almacenamiento` | `basurero_historial` | `PutItem` |
| `basurero_alexa` | `basurero_usuarios` | `GetItem`, `PutItem`, `DeleteItem`, `UpdateItem` |

## Verificación

Ruta:

`DynamoDB → Explorar elementos → Seleccionar tabla`

Se confirmó el almacenamiento de telemetrías de ambos Things y las vinculaciones de usuarios.

No se debe confundir el estado actual del Shadow con el historial almacenado en DynamoDB.

## Patrones de acceso

- Obtener historial de un Thing por su clave.
- Consultar registros ordenados temporalmente.
- Resolver `userId → thingName`.
- Guardar o eliminar una vinculación.
- Incrementar el contador de comandos.

No se configuró un índice secundario como parte de las pruebas descritas. El uso de TTL queda como posible mejora.

## Referencia

[Documentación de DynamoDB](https://docs.aws.amazon.com/amazondynamodb/latest/developerguide/Introduction.html)
