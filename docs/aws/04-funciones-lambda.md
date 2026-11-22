
# Funciones AWS Lambda

[Volver al índice](README.md)

## Configuración

**Región:** `us-east-2`  
**Lenguaje:** Python  
**Runtime configurado:** Python 3.12

Se utilizaron tres Lambdas para separar responsabilidades y permisos.

| Función | Disparador | Destino principal |
|---|---|---|
| `basurero_almacenamiento` | Reglas de telemetría y eventos | DynamoDB |
| `basurero_control_capacidad` | Regla de capacidad | Device Shadow |
| `basurero_alexa` | Alexa Skills Kit | DynamoDB e IoT Core |

## 1. basurero_almacenamiento

Procesa mensajes recibidos desde las reglas IoT.

Responsabilidades:
- Validar datos de entrada.
- Distinguir `TELEMETRIA` y `EVENTO`.
- Identificar el `thingName`.
- Agregar o conservar marcas temporales.
- Guardar registros en `basurero_historial`.

**Permiso principal:** `dynamodb:PutItem`.

**Código:** [basurero_almacenamiento.py](lambdas/basurero_almacenamiento.py)

## 2. basurero_control_capacidad

Recibe el nombre del Thing y su porcentaje de llenado.

Consulta el Shadow y aplica una histéresis de diez puntos porcentuales.

Con umbral predeterminado de 90 %:

| Condición | Resultado |
|---|---|
| `fillPercent >= 90` | Solicitar bloqueo |
| `80 < fillPercent < 90` | Mantener estado |
| `fillPercent <= 80` | Solicitar desbloqueo |

Si el umbral cambia, también cambia el límite inferior (`umbral - 10`).

La Lambda modifica `desired.capacityLock`, no el estado físico reportado.

**Permisos principales:**
- `iot:GetThingShadow`
- `iot:UpdateThingShadow`

**Código:** [basurero_control_capacidad.py](lambdas/basurero_control_capacidad.py)

## 3. basurero_alexa

Atiende las solicitudes de la Alexa Skill.

Responsabilidades:
- Identificar al usuario mediante `userId`.
- Vincular y desvincular dispositivos.
- Obtener el `thingName` desde DynamoDB.
- Consultar `reported` del Shadow.
- Actualizar `desired` para enviar órdenes.
- Generar identificadores persistentes de comandos.
- Construir respuestas compatibles con Alexa.

**Variables de entorno:**

| Variable | Uso |
|---|---|
| `TABLE_NAME` | Tabla `basurero_usuarios` |
| `IOT_ENDPOINT` | Endpoint IoT Data |
| `PAIRING_CODES_JSON` | Códigos asociados a Things |
| `SKILL_ID` | Identificador de la Skill |

**Servicios utilizados:** DynamoDB, AWS IoT y AWS IoT Data.

**Código:** [basurero_alexa.py](lambdas/basurero_alexa.py)

## ¿Por qué tres Lambdas?

La separación permite:
1. Cambiar el almacenamiento sin afectar Alexa.
2. Mantener la lógica de capacidad independiente.
3. Asignar permisos IAM específicos.
4. Diagnosticar errores mediante CloudWatch.
5. Reutilizar funciones con distintos Things.

## Importante sobre los comandos

La respuesta de Alexa confirma que se envió la solicitud.

El ESP32 aplica la acción y después actualiza `reported`.

Un comando puede ser rechazado localmente por bloqueo de capacidad u otra condición de seguridad.

## Supervisión

Los registros de cada Lambda se consultan desde:

`AWS Lambda → Función → Monitor → CloudWatch Logs`

Los permisos se explican en [08-permisos-iam.md](08-permisos-iam.md).

**Nota:** los archivos Python deben copiarse desde las versiones finales desplegadas en AWS, no reconstruirse a partir de pseudocódigo.
