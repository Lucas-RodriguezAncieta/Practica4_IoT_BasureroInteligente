
# Seguridad, roles y políticas IAM

[Volver al índice](README.md)

## Objetivo

Documentar los permisos utilizados por AWS IoT Core y las funciones Lambda, aplicando separación de responsabilidades y el principio de mínimo privilegio.

## Política IoT del ESP32

**Nombre:** `BasureroP4IoTPolicy`

Esta política se asocia al certificado del dispositivo físico.

Controla operaciones MQTT:
- `iot:Connect`
- `iot:Publish`
- `iot:Subscribe`
- `iot:Receive`

La conexión utiliza el Client ID `basurero-grupo3-01`.

El ejemplo de política IoT se encuentra en la raíz del repositorio:

[aws-iot-policy.example.json](../../aws-iot-policy.example.json)

Una política IoT de certificado no equivale a un rol IAM de Lambda.

## Permisos de las Lambdas

| Lambda | Acciones principales | Recurso |
|---|---|---|
| `basurero_almacenamiento` | `dynamodb:PutItem` | `basurero_historial` |
| `basurero_control_capacidad` | `iot:GetThingShadow`, `iot:UpdateThingShadow` | Things autorizados |
| `basurero_alexa` | `dynamodb:GetItem`, `PutItem`, `UpdateItem`, `DeleteItem` | `basurero_usuarios` |
| `basurero_alexa` | `iot:DescribeThing`, `GetThingShadow`, `UpdateThingShadow` | Things autorizados |

Las funciones también requieren permisos apropiados para escribir logs en CloudWatch.

## Modificación para Multi-Thing

Inicialmente, la política de capacidad autorizaba únicamente:

```text
basurero-grupo3-01
```

Se actualizó `BasureroControlCapacidadRolePolicy`, asociada al rol `BasureroControlCapacidadRole`, para incluir:

```text
basurero-grupo3-01
basurero-grupo3-02
```

Las acciones necesarias son:

```text
iot:GetThingShadow
iot:UpdateThingShadow
```

La política de Alexa ya contemplaba Things cuyos nombres comienzan con `basurero-`.

No fue necesario modificar la política del certificado del ESP32 para incorporar el Thing simulado.

## Permisos de invocación

Debe distinguirse entre:

**Política de ejecución:** qué puede hacer una Lambda cuando se ejecuta.

**Permiso de invocación:** qué servicio puede ejecutar esa Lambda.

En nuestro proyecto:
- AWS IoT Core invoca almacenamiento y capacidad.
- Alexa Skills Kit invoca `basurero_alexa`.

La integración de Alexa se restringió al identificador de la Skill correspondiente.

## Configuraciones de referencia

- [IAM almacenamiento](config/iam/almacenamiento.example.json)
- [IAM capacidad](config/iam/capacidad.example.json)
- [IAM Alexa](config/iam/alexa.example.json)

Estos JSON deben representar las acciones y recursos de las políticas reales, sin incluir información sensible.

## Buenas prácticas

- Restringir acciones a los recursos necesarios.
- Evitar `Resource: "*"`, salvo justificación.
- No publicar access keys, claves privadas ni secretos.
- No almacenar credenciales de AWS en el firmware.
- No compartir certificados de dispositivo entre clientes independientes.
- Revisar permisos nuevos antes de incorporar otro Thing.

## Verificación

Las políticas y roles pueden consultarse desde:

`AWS Lambda → Función → Configuración → Permisos → Rol de ejecución`

También desde:

`AWS IAM → Roles / Políticas`
