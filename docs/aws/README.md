
# Documentación AWS y Alexa — Basurero Inteligente

## Descripción

Esta carpeta documenta la infraestructura en la nube desarrollada para la Práctica 4 de IoT (SIS-234).

El proyecto integra un ESP32 con AWS IoT Core, Device Shadow, AWS IoT Rules, AWS Lambda, Amazon DynamoDB y Alexa Skills Kit.

La arquitectura permite:
- Recibir mediciones y eventos de los dispositivos.
- Almacenar un historial separado por Thing.
- Aplicar bloqueo automático por capacidad.
- Controlar y consultar el basurero mediante Alexa.
- Vincular diferentes usuarios con diferentes Things.

## Recursos implementados

| Servicio | Recursos |
|---|---|
| AWS IoT Core | 2 Things y sus Shadows clásicos |
| AWS IoT Rules | 3 reglas |
| AWS Lambda | 3 funciones Python |
| DynamoDB | 2 tablas |
| Alexa | 1 Skill |
| IAM | Roles y políticas para las Lambdas |

**Región AWS:** `us-east-2` (Ohio).

## Índice

1. [Arquitectura AWS](01-arquitectura-aws.md)
2. [IoT Core y Device Shadow](02-iot-core-y-device-shadow.md)
3. [Reglas IoT y MQTT](03-reglas-iot-y-mqtt.md)
4. [Funciones Lambda](04-funciones-lambda.md)
5. [DynamoDB](05-dynamodb.md)
6. [Alexa Skill](06-alexa-skill.md)
7. [Vinculación Multi-Thing](07-vinculacion-multithing.md)
8. [Permisos IAM](08-permisos-iam.md)

## Configuraciones y código

| Carpeta | Contenido |
|---|---|
| `config/reglas/` | Consultas SQL de las tres reglas |
| `config/alexa/` | Modelo de interacción exportado |
| `config/iam/` | Ejemplos de políticas IAM |
| `config/shadows/` | Ejemplos de Device Shadow |
| `config/pruebas/` | Eventos de prueba |
| `lambdas/` | Código fuente Python de las tres Lambdas |
| `evidencias/` | Capturas de configuración y resultados |

Las configuraciones se incorporarán progresivamente y deberán corresponder a los recursos desplegados.

## Alcance y seguridad

Esta documentación corresponde a la integración AWS y Alexa. El firmware y el informe general se mantienen en sus ubicaciones originales.

No se deben publicar claves privadas, credenciales WiFi, access keys ni códigos de vinculación activos.

Las pruebas realizadas con un segundo Thing y una segunda identidad simulada se identifican expresamente como simulaciones.
