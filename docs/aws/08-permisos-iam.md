
# Seguridad, roles y políticas IAM

[Volver al índice](README.md)

## 1. Objetivo

Documentar la seguridad y los permisos utilizados en la integración de AWS IoT Core, Lambda, DynamoDB y Alexa para el proyecto **Basurero Inteligente — Práctica 4**.

Se aplica el principio de mínimo privilegio, asignando a cada componente únicamente las operaciones necesarias.

**Región AWS:** `us-east-2` (Ohio).

## 2. Autenticación del ESP32

El dispositivo físico `basurero-grupo3-01` se conecta a AWS IoT Core mediante MQTT sobre TLS, utilizando:

- Certificado digital X.509.
- Clave privada del dispositivo.
- Endpoint AWS IoT Data-ATS.
- Política de seguridad `BasureroP4IoTPolicy`.

La política IoT controla operaciones como:

| Acción | Finalidad |
|---|---|
| `iot:Connect` | Conectarse al broker MQTT |
| `iot:Publish` | Publicar mensajes |
| `iot:Subscribe` | Suscribirse a tópicos |
| `iot:Receive` | Recibir mensajes |

El Client ID autorizado para el ESP32 es `basurero-grupo3-01`.

**Archivo de referencia:**

[aws-iot-policy.example.json](../../aws-iot-policy.example.json)

Esta política corresponde al certificado del ESP32 y es independiente de los roles IAM de Lambda.

## 3. Roles IAM de las funciones Lambda

Cada Lambda utiliza un rol de ejecución para acceder a los servicios AWS necesarios.

| Lambda | Servicio | Acciones principales |
|---|---|---|
| `basurero_almacenamiento` | DynamoDB | `PutItem` |
| `basurero_control_capacidad` | AWS IoT Core | `GetThingShadow`, `UpdateThingShadow` |
| `basurero_alexa` | DynamoDB e IoT Core | Gestión de usuarios y Shadows |

Las políticas personalizadas de las tres funciones fueron verificadas directamente desde AWS IAM.

## 4. Política de almacenamiento

**Función:** `basurero_almacenamiento`

**Identificador de instrucción:** `GuardarHistorialBasurero`

| Elemento | Configuración verificada |
|---|---|
| Efecto | `Allow` |
| Acción | `dynamodb:PutItem` |
| Recurso | Tabla `basurero_historial` |
| Región | `us-east-2` |

La política permite guardar telemetrías y eventos recibidos mediante las reglas de AWS IoT Core.

No concede permisos de lectura, modificación o eliminación de registros mediante esta instrucción.

**Archivo de referencia:**

[almacenamiento.example.json](config/iam/almacenamiento.example.json)

## 5. Política de control de capacidad

**Función:** `basurero_control_capacidad`

**Rol:** `BasureroControlCapacidadRole`

**Política:** `BasureroControlCapacidadRolePolicy`

**Identificador de instrucción:** `ConsultarYActualizarShadow`

### Acciones autorizadas

| Acción | Finalidad |
|---|---|
| `iot:GetThingShadow` | Consultar el estado del Thing |
| `iot:UpdateThingShadow` | Actualizar el estado deseado |

### Recursos autorizados

- `basurero-grupo3-01`
- `basurero-grupo3-02`

La política contiene los ARN específicos de ambos Things.

Esto permite que la Lambda procese las mediciones de distintos dispositivos y actualice `desired.capacityLock` según el nivel de llenado.

### Relación con Multi-Thing

Inicialmente, la política autorizaba únicamente el Thing 01.

Posteriormente, se amplió para incorporar el Thing 02, utilizado en las pruebas de Multi-Thing e histéresis.

**Archivo de referencia:**

[capacidad.example.json](config/iam/capacidad.example.json)

## 6. Política de Alexa

**Función:** `basurero_alexa`

La política verificada contiene dos instrucciones.

### 6.1. GestionarVinculacionesAlexa

Autoriza operaciones sobre la tabla `basurero_usuarios`.

| Acción | Finalidad |
|---|---|
| `dynamodb:GetItem` | Consultar vinculación |
| `dynamodb:PutItem` | Crear vinculación |
| `dynamodb:UpdateItem` | Actualizar contadores y atributos |
| `dynamodb:DeleteItem` | Eliminar vinculación |

Estas operaciones permiten administrar la relación entre `userId` y `thingName`.

También permiten mantener el contador de identificadores de comandos.

### 6.2. ConsultarYControlarBasureros

Autoriza operaciones sobre los Things cuyo nombre comienza con `basurero-`.

| Acción | Finalidad |
|---|---|
| `iot:DescribeThing` | Verificar que exista el Thing |
| `iot:GetThingShadow` | Consultar el estado reportado |
| `iot:UpdateThingShadow` | Solicitar cambios de configuración y comandos |

**Patrón de recursos:**

```text
arn:aws:iot:us-east-2:AWS_ACCOUNT_ID:thing/basurero-*
```

El uso de `basurero-*` permite trabajar con varios Things sin configurar un ARN diferente para cada dispositivo.

La Lambda identifica el Thing correspondiente al usuario mediante DynamoDB.

**Archivo de referencia:**

[alexa.example.json](config/iam/alexa.example.json)

## 7. Permisos de invocación y registros

Los permisos de ejecución IAM son diferentes de los permisos necesarios para invocar una función Lambda.

En el proyecto:

| Servicio invocador | Función destino |
|---|---|
| AWS IoT Rules | `basurero_almacenamiento` |
| AWS IoT Rules | `basurero_control_capacidad` |
| Alexa Skills Kit | `basurero_alexa` |

La integración de Alexa fue configurada con una restricción al identificador de la Skill.

Los registros de ejecución se consultan mediante Amazon CloudWatch Logs.

Los permisos de CloudWatch y las políticas de invocación se administran por separado de las tres instrucciones IAM personalizadas verificadas.

## 8. Resultado de la verificación

Se compararon las políticas IAM reales con los archivos de ejemplo del repositorio.

| Función | Verificación | Resultado |
|---|---|---|
| `basurero_almacenamiento` | Acción `PutItem` y tabla correcta | Conforme |
| `basurero_control_capacidad` | Lectura y actualización de ambos Shadows | Conforme |
| `basurero_alexa` | DynamoDB, IoT Core y patrón Multi-Thing | Conforme |

**Resultado:** las tres políticas personalizadas coinciden en acciones y recursos con los ejemplos documentados.

Los archivos `.example.json` utilizan `AWS_ACCOUNT_ID` como marcador de posición y no incluyen credenciales.

Esta revisión confirma las políticas proporcionadas, pero no constituye una auditoría de todos los permisos efectivos de la cuenta AWS.

## 9. Buenas prácticas de seguridad

- Aplicar el principio de mínimo privilegio.
- Restringir los permisos a los recursos necesarios.
- Proteger certificados y claves privadas del ESP32.
- No publicar credenciales ni códigos de vinculación activos.
- No almacenar access keys de AWS dentro del firmware.
- Diferenciar las políticas de certificados IoT y los roles IAM.
- Revisar los permisos antes de incorporar nuevos Things.

## 10. Referencias

- [Documentación de AWS IAM](https://docs.aws.amazon.com/IAM/latest/UserGuide/introduction.html)
- [Seguridad de AWS IoT Core](https://docs.aws.amazon.com/iot/latest/developerguide/iot-security.html)
- [Permisos de AWS Lambda](https://docs.aws.amazon.com/lambda/latest/dg/lambda-permissions.html)
- [Arquitectura del proyecto](01-arquitectura-aws.md)
- [Vinculación Multi-Thing](07-vinculacion-multithing.md)
