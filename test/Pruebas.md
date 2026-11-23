# Plan de Pruebas y Validación – Basurero Inteligente

| ID | Prueba | Procedimiento | Resultado esperado | Estado |
|---|---|---|---|---|
| T01 | Detección de proximidad (IR1) | Acercar la mano al IR1 y repetir 10 veces. | Detecta la presencia y abre la tapa correctamente en modo AUTO (10/10 intentos). | PENDIENTE |
| T02 | Detección continua | Mantener la mano frente al IR1 durante varios segundos. | No genera aperturas repetitivas ni activaciones inesperadas. | PENDIENTE |
| T03 | Modo MANUAL | Activar MANUAL, acercar la mano al IR1 y probar comandos manuales. | IR1 no activa la apertura automática; los comandos manuales funcionan. | PARCIAL: demostración de modos documentada; falta registrar el procedimiento completo y evidencia. |
| T04 | Cierre temporizado | Abrir la tapa y medir el tiempo de cierre en 5 intentos. | Respeta el tiempo mínimo configurado antes de cerrar. | PENDIENTE |
| T05 | Protección IR2 antes del cierre | Activar IR2 antes de que comience el cierre. | Impide el cierre mientras detecta un obstáculo. | PENDIENTE |
| T06 | Obstáculo durante el cierre | Activar IR2 mientras la tapa se está cerrando. | Detiene el cierre y reacciona según la lógica de seguridad. | PENDIENTE |
| T07 | Estabilidad mecánica | Realizar 20 ciclos de apertura y cierre. | Sin atascos, reinicios ni calentamiento anormal. | PENDIENTE |
| T08 | Calibración del llenado | Medir vacío, nivel intermedio y nivel alto; realizar 10 lecturas por nivel. | Error de medición dentro del objetivo de ±10 puntos porcentuales. | PENDIENTE |
| T09 | Mediciones inválidas | Provocar lecturas sin eco o fuera de rango. | Identifica mediciones inválidas y evita generar porcentajes incorrectos. | PENDIENTE |
| T10 | Funcionamiento offline | Desconectar Internet y probar apertura, cierre, IR2 y medición. Después reconectar. | Funciones locales operativas y recuperación de la conexión con AWS. | PARCIAL: se documentó una reconexión; falta prueba controlada offline y recuperación. |
| T11 | Conexión ESP32–AWS | Conectar el dispositivo a Wi-Fi y AWS IoT Core y publicar telemetría. | Conexión estable y mensajes recibidos en el topic configurado. | PARCIAL: conexión y publicaciones documentadas; falta medir estabilidad y registrar ejecución formal. |
| T12 | Device Shadow | Modificar un parámetro en `desired` y verificar `reported`. | El ESP32 aplica el cambio y reporta el estado actualizado. | PARCIAL: Shadow y sincronización observados; falta evidencia correlacionada del cambio solicitado y aplicado. |
| T13 | IoT Rule y Lambda | Enviar una lectura y verificar la ejecución de Rule y Lambda. | El mensaje es detectado y procesado correctamente. | PARCIAL: reglas y disparadores documentados; falta registro correlacionado de una invocación. |
| T14 | Almacenamiento DynamoDB | Enviar 10 lecturas y 5 eventos y revisar la tabla. | Los registros se almacenan con los campos correspondientes. | PARCIAL: historial y eventos observados; falta comprobar la muestra específica de 10 lecturas y 5 eventos. |
| T15 | Validación de mensajes | Enviar 5 mensajes inválidos (valores fuera de rango, campos faltantes, tipos incorrectos). | Los mensajes inválidos son rechazados o gestionados sin generar datos incorrectos. | PENDIENTE |
| T16 | Integridad y latencia | Medir mensajes válidos enviados frente a almacenamiento. | Integridad ≥99 % y latencia ≤5 segundos. | APROBADA EN NUBE SIMULADA: 100/100 mensajes; máximo 1,135 s en otra muestra de 10. Falta validar la ruta desde el ESP32 físico. |
| T17 | Separación de usuarios | Utilizar 2 usuarios vinculados a 2 Things distintos y realizar consultas. | Cada usuario accede únicamente a su dispositivo. | PARCIAL: un usuario Alexa y una identidad simulada; falta evidencia completa de denegación y aislamiento. |
| T18 | Consultas mediante Alexa | Consultar nivel de llenado, estado de tapa y modo. | Alexa responde con datos reales y reconoce datos no disponibles. | PARCIAL: consultas descritas en el registro; falta evidencia de todos los casos. |
| T19 | Comandos por voz y seguridad | Ordenar abrir, cerrar y cambiar modo; probar IR2 y usuario no vinculado. | Comandos autorizados funcionan, IR2 protege y usuarios no vinculados no acceden. | PARCIAL: se describe control físico por Alexa; faltan pruebas completas de IR2 y usuario no vinculado. |
| T20 | Bloqueo por llenado (Edge/Cloud) | Simular el umbral de bloqueo, probar apertura automática y reducir llenado hasta desbloqueo. | Lambda aplica el bloqueo, el ESP32 lo respeta y el desbloqueo sigue la histéresis configurada. | PARCIAL: secuencia 95/85/80 documentada para Thing simulado; falta validar el ciclo completo en el ESP32 físico. |

## Registro de métricas

| Métrica | Objetivo | Resultado obtenido |
|---|---|---|
| Respuesta de apertura Edge | ≤500 ms | Por medir |
| Respuesta del sensor IR2 | Registrar tiempo de respuesta | Por medir |
| Respuesta comando Cloud | ≤5 s | Por medir |
| Latencia de almacenamiento | ≤5 s | Promedio 0,326 s; máximo 1,135 s en 10 mensajes simulados. |
| Integridad de almacenamiento | ≥99 % | 100 %: 100 de 100 mensajes simulados identificados en DynamoDB. |
| Error de medición del llenado | ≤10 puntos porcentuales | Por medir |
| Variabilidad de mediciones | ≤5 pp, si es factible | Por medir |
| Recuperación Wi-Fi/MQTT | Registrar tiempo de recuperación | Por medir |
| Pérdida de eventos offline | Registrar cantidad perdida | Por medir |
| Ciclos mecánicos | 20/20 exitosos | Por medir |
| Rechazo de mensajes inválidos | 100 % | Por medir |
| Costo mensual de AWS | Estimación documentada | Proyección preliminar ≈0,55 USD/mes en el informe; faltan fuentes de tarifas y contraste con la cuenta. |

## Resultados y evidencias registradas

### T16 — Integridad y latencia en la nube

El informe documenta ejecuciones del 23/11/2026 con `basurero-grupo3-02` simulado, publicadas desde AWS CloudShell con QoS 1:

- **Integridad:** 100 publicaciones aceptadas, 100 registros encontrados, cero faltantes y cero duplicados reportados. El resultado observado es 100 % para esa muestra. Evidencias: [datos de la ejecución](../docs/aws/evidencias/mediciones/prueba_integridad_100.json) y [resultado de integridad](../docs/aws/evidencias/mediciones/resultado_integridad_100.json).
- **Latencia:** en una ejecución independiente, se encontraron 10 de 10 mensajes; la media fue 0,326 s y el máximo 1,135 s. El tiempo comprende desde el inicio de publicación hasta encontrar el registro mediante consultas a DynamoDB. Evidencia: [resultados individuales](../docs/aws/evidencias/mediciones/resultado_latencia_10.json).

Ambas muestras alcanzaron los objetivos de T16 **en las condiciones simuladas documentadas**. No demuestran la integridad ni la latencia desde el ESP32 físico, cuya publicación habitual utiliza QoS 0.

### Observaciones parciales de otras pruebas

- **T03, T11, T12, T18 y T19:** el [registro de configuración y demostraciones, §12–13](../CONFIGURACION_AWS_ALEXA_BASURERO_INTELIGENTE_P4.md#13-pruebas-funcionales-realizadas-y-estado-de-evidencia) describe conexión, Shadow, modos y control físico mediante Alexa. Estas observaciones no incluyen una campaña formal completa ni todas las evidencias por caso.
- **T10:** el mismo registro describe una reconexión MQTT; no registra una prueba controlada de funcionamiento sin red ni tiempos de recuperación.
- **T13–T14:** las capturas muestran [reglas activas](../docs/aws/evidencias/iot-core/02-reglas-iot.png), [disparadores de almacenamiento](../docs/aws/evidencias/lambda/02-disparadores-almacenamiento.png) y [registros de telemetría y eventos](../docs/aws/evidencias/dynamodb/02-historial.png). Por sí solas no correlacionan una invocación con la muestra completa requerida.
- **T17:** la [captura de vinculaciones](../docs/aws/evidencias/multithing/01-vinculaciones.png) muestra asociaciones con ambos Things, y el [Shadow del Thing 02](../docs/aws/evidencias/multithing/03-shadow-thing02.png) muestra 35 % simulado. La segunda identidad y el segundo Thing son simulados.
- **T20:** el registro AWS describe que, con umbral de 90 %, las telemetrías simuladas de 95 %, 85 % y 80 % solicitaron bloqueo, conservaron el bloqueo y solicitaron desbloqueo, respectivamente. No demuestran por sí solas el ciclo completo de bloqueo y liberación en el equipo físico.
- **Consulta complementaria, no prueba de integridad:** se registraron 118 eventos existentes: 92 `edge`, 21 `voz` y 5 `nube`, ninguno sin origen. [Resultado de la consulta](../docs/aws/evidencias/mediciones/resultado_eventos_origen.json). El conteo no identifica todos los eventos generados ni acredita entrega completa.

Las demás pruebas y métricas permanecen pendientes de registro. La alimentación USB del servo se ha observado funcionando, según comunicación del equipo; aún no hay una serie documentada de 20 ciclos para aprobar T07.
