# Plan de Pruebas y Validación – Basurero Inteligente

| ID | Prueba | Procedimiento | Resultado esperado | Estado |
|---|---|---|---|---|
| T01 | Detección de proximidad (IR1) | Acercar la mano al IR1 y repetir 10 veces. | Detecta la presencia y abre la tapa correctamente en modo AUTO (10/10 intentos). | PENDIENTE |
| T02 | Detección continua | Mantener la mano frente al IR1 durante varios segundos. | No genera aperturas repetitivas ni activaciones inesperadas. | PENDIENTE |
| T03 | Modo MANUAL | Activar MANUAL, acercar la mano al IR1 y probar comandos manuales. | IR1 no activa la apertura automática; los comandos manuales funcionan. | PENDIENTE |
| T04 | Cierre temporizado | Abrir la tapa y medir el tiempo de cierre en 5 intentos. | Respeta el tiempo mínimo configurado antes de cerrar. | PENDIENTE |
| T05 | Protección IR2 antes del cierre | Activar IR2 antes de que comience el cierre. | Impide el cierre mientras detecta un obstáculo. | PENDIENTE |
| T06 | Obstáculo durante el cierre | Activar IR2 mientras la tapa se está cerrando. | Detiene el cierre y reacciona según la lógica de seguridad. | PENDIENTE |
| T07 | Estabilidad mecánica | Realizar 20 ciclos de apertura y cierre. | Sin atascos, reinicios ni calentamiento anormal. | PENDIENTE |
| T08 | Calibración del llenado | Medir vacío, nivel intermedio y nivel alto; realizar 10 lecturas por nivel. | Error de medición dentro del objetivo de ±10 puntos porcentuales. | PENDIENTE |
| T09 | Mediciones inválidas | Provocar lecturas sin eco o fuera de rango. | Identifica mediciones inválidas y evita generar porcentajes incorrectos. | PENDIENTE |
| T10 | Funcionamiento offline | Desconectar Internet y probar apertura, cierre, IR2 y medición. Después reconectar. | Funciones locales operativas y recuperación de la conexión con AWS. | PENDIENTE |
| T11 | Conexión ESP32–AWS | Conectar el dispositivo a Wi-Fi y AWS IoT Core y publicar telemetría. | Conexión estable y mensajes recibidos en el topic configurado. | PENDIENTE |
| T12 | Device Shadow | Modificar un parámetro en `desired` y verificar `reported`. | El ESP32 aplica el cambio y reporta el estado actualizado. | PENDIENTE |
| T13 | IoT Rule y Lambda | Enviar una lectura y verificar la ejecución de Rule y Lambda. | El mensaje es detectado y procesado correctamente. | PENDIENTE |
| T14 | Almacenamiento DynamoDB | Enviar 10 lecturas y 5 eventos y revisar la tabla. | Los registros se almacenan con los campos correspondientes. | PENDIENTE |
| T15 | Validación de mensajes | Enviar 5 mensajes inválidos (valores fuera de rango, campos faltantes, tipos incorrectos). | Los mensajes inválidos son rechazados o gestionados sin generar datos incorrectos. | PENDIENTE |
| T16 | Integridad y latencia | Enviar 20 mensajes válidos y medir envío frente a almacenamiento. | Integridad ≥99 % y latencia ≤5 segundos. | PENDIENTE |
| T17 | Separación de usuarios | Utilizar 2 usuarios vinculados a 2 Things distintos y realizar consultas. | Cada usuario accede únicamente a su dispositivo. | PENDIENTE |
| T18 | Consultas mediante Alexa | Consultar nivel de llenado, estado de tapa y modo. | Alexa responde con datos reales y reconoce datos no disponibles. | PENDIENTE |
| T19 | Comandos por voz y seguridad | Ordenar abrir, cerrar y cambiar modo; probar IR2 y usuario no vinculado. | Comandos autorizados funcionan, IR2 protege y usuarios no vinculados no acceden. | PENDIENTE |
| T20 | Bloqueo por llenado (Edge/Cloud) | Simular el umbral de bloqueo, probar apertura automática y reducir llenado hasta desbloqueo. | Lambda aplica el bloqueo, el ESP32 lo respeta y el desbloqueo sigue la histéresis configurada. | PENDIENTE |

## Registro de métricas

| Métrica | Objetivo | Resultado obtenido |
|---|---|---|
| Respuesta de apertura Edge | ≤500 ms | Por medir |
| Respuesta del sensor IR2 | Registrar tiempo de respuesta | Por medir |
| Respuesta comando Cloud | ≤5 s | Por medir |
| Latencia de almacenamiento | ≤5 s | Por medir |
| Integridad de almacenamiento | ≥99 % | Por medir |
| Error de medición del llenado | ≤10 puntos porcentuales | Por medir |
| Variabilidad de mediciones | ≤5 pp, si es factible | Por medir |
| Recuperación Wi-Fi/MQTT | Registrar tiempo de recuperación | Por medir |
| Pérdida de eventos offline | Registrar cantidad perdida | Por medir |
| Ciclos mecánicos | 20/20 exitosos | Por medir |
| Rechazo de mensajes inválidos | 100 % | Por medir |
| Costo mensual de AWS | Estimación documentada | Por calcular |
