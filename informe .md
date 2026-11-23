
Práctica 4 — Basurero inteligente conectado a AWS IoT Core y Alexa

Asignatura: SIS-234 · Ingeniería de Sistemas
Equipo: Grupo 3 · Lucas Rodriguez - Josue Camacho - Pablo Raya
Fecha y versión de entrega: 24-11-26

1. Caso de uso y requerimientos
1.1 Problema y solución

El basurero inteligente facilita el depósito de residuos sin tocar la tapa y permite consultar a distancia el estado y el llenado estimado. Un ESP32 lee dos sensores infrarrojos y un ultrasónico, acciona una tapa mediante un SG90 y comunica su estado a AWS. Una skill de Alexa consulta el último estado y solicita cambios mediante el Shadow del Thing asociado al usuario.

La Práctica 4 amplía el control por voz y la vinculación usuario–Thing de la práctica anterior con control autónomo local, telemetría, historial y una regla de capacidad en la nube. La documentación disponible no describe con suficiente precisión el montaje físico anterior para atribuir componentes concretos a esa versión.
Componente	Cantidad	Función y justificación
ESP32 DOIT DevKit V1	1	Procesamiento local, GPIO y comunicación WiFi.
IR1 de proximidad	1	Detecta la aproximación para abrir sin contacto.
IR2 de obstáculos	1	Detecta obstáculos en la zona supervisada durante el cierre.
HC-SR04	1	La distancia a los residuos permite estimar el nivel de un recipiente de altura física aproximada de 30 cm; su uso responde al caso de uso.
SG90	1	Actuador no LED que mueve la tapa; funciona con alimentación de 5 V.

IR2 no detecta necesariamente todos los obstáculos; el sistema no debe considerarse una protección contra atrapamientos. El estado de la tapa es estimado, pues el servo no aporta realimentación de posición.
1.2 Requerimientos funcionales y trazabilidad
ID	Requerimiento verificable	Diseño o implementación	Prueba / evidencia y estado
F01	Capturar IR1, IR2 y distancia; estimar y reportar llenado.	
Sensores,
Shadow.	T01, T08 y T09 en
plan, pendientes de registro formal; el registro AWS describe lecturas físicas.
F02	Abrir por proximidad en automático, sin repetir por detección sostenida.	
SmartBin, IR1 filtrado.	T01–T02 pendientes.
F03	Cerrar tras el retardo configurado, con IR2 libre; reabrir ante obstáculo durante cierre.	
SmartBin,
IR2.	T04–T06 pendientes.
F04	Distinguir modos automático y manual y aceptar comandos locales o del Shadow.	
SmartBin,
main.	Registro AWS informa demostración de ambos modos; T03 y T19 pendientes formalmente.
F05	Sincronizar configuración, sensores y estado estimado por Shadow; conectar por MQTT/TLS.	
AWSIoTManager,
ShadowManager.	Registro AWS informa conexión y aceptación de reported; T11–T12 pendientes formalmente.
F06	Vincular usuarios a Things y consultar/controlar por Alexa sin Thing fijo.	Lambda y tabla de usuarios descritas en
configuración AWS.	Una cuenta Alexa y otra identidad simulada; T17–T19 pendientes formalmente.
F07	Enviar telemetría y eventos a reglas y guardar histórico por Thing.	Tres reglas y Lambda de almacenamiento, §3.7.	Registro AWS describe tres lecturas simuladas almacenadas; T13–T16 pendientes formalmente.
F08	Aplicar bloqueo por capacidad en nube con histéresis y respetarlo en Edge.	Regla y Lambda de capacidad;
SmartBin.	Registro AWS describe 95/85/80 % en Thing simulado; T20 pendiente formalmente.
F09	Consultar y modificar modo y parámetros desde Alexa.	Intents de modo, umbral y retardo, §3.9.	Registro AWS documenta solicitudes y respuestas; faltan capturas actuales de reported.
F10	Diseñar reportes para decisiones a partir del histórico.	Mockups en §3.10.	Diseño ilustrativo; consultas y datos reales por verificar.

Los requisitos funcionales de esta práctica se identifican F01–F10 para evitar confundirlos con los requisitos heredados de la Práctica 3. Antes de entregar, transcribir literalmente RF01–RF09 de la Práctica 3 y agregar una columna que indique si cada uno se mantiene, modifica o amplía. No se afirma que F01–F10 sustituyan automáticamente todos los requisitos originales de la consigna.
1.3.1 Trazabilidad de requerimientos al sistema
Función	Componente responsable	Dato/acción observable	Prueba relacionada
Detectar proximidad	IR1 + SmartBin	Estado IR y solicitud de apertura	T01–T02
Evitar cierre con obstáculo	IR2 + SmartBin + servo	Mantener/reabrir tapa	T05–T06
Control manual/automático	Firmware + Shadow + Alexa	Modo actual y órdenes	T03, T12, T19
Medir llenado	HC-SR04 + cálculo en Edge	Distancia y porcentaje válido	T08–T09
Reportar datos	ESP32 + MQTT + IoT Core	Telemetría y evento en tópico	T11, T13–T16
Mantener historial	Rule + Lambda + DynamoDB	Item por Thing y tiempo	T13–T16
Consultar/controlar por voz	Skill + Lambda + Shadow	Respuesta y comando	T17–T19
Aplicar bloqueo por llenado	Lambda nube + Shadow + Edge	capacityLock y comportamiento	T20
Operar sin Internet	Lógica Edge	Apertura, cierre e IR2 local	T10
Mostrar reportes	Mockup/consulta histórica	Serie de llenado y eventos	T14, consulta de datos
1.4 Requerimientos no funcionales

Los valores siguientes son objetivos, no resultados medidos. El procedimiento y registro se detallan en §5 y en el
plan de pruebas.
Atributo	Objetivo o condición	Método
Integridad de almacenamiento	≥99 % de mensajes válidos de una ventana definida.	Comparar identificadores únicos publicados con elementos en DynamoDB; registrar duplicados y errores.
Latencia de almacenamiento	≤5 s desde publicación hasta persistencia observada.	Medir ambos extremos con relojes sincronizados y registrar máximo y distribución.
Respuesta Edge	≤500 ms desde detección IR1 estable hasta inicio observable de apertura.	Registrar detección y movimiento físico; declarar resolución del instrumento.
Respuesta de comando Cloud	≤5 s desde solicitud hasta inicio observable de movimiento, con conexión estable.	Marcar solicitud y movimiento; no usar la respuesta de Alexa como confirmación física.
Exactitud de llenado	Error ≤10 puntos porcentuales frente a una referencia definida.	Diez lecturas en vacío, nivel intermedio y alto; comparar con referencia.
Disponibilidad local	Mantener las reglas Edge durante pérdida de red.	Interrumpir WiFi, repetir acciones y registrar eventos perdidos y reconexión.
Seguridad y acceso	TLS con certificado de servidor y cliente, sin publicar secretos; mínimo privilegio.	Revisar configuración y políticas efectivas.
Costo	Estimar uso mensual y contrastar precios y límites vigentes.	Calcular volumen desde frecuencia y consultar tarifas aplicables.
2. Análisis del objeto inteligente
2.1 Bloques y conexiones
IR1: proximidadESP32: control EdgeIR2: obstáculoHC-SR04: distanciaSG90: tapaAWS IoT CoreWiFi, MQTT/TLS
Señal	GPIO ESP32	Observaciones
IR1 salida digital	27	Activo en LOW en la configuración actual.
IR2 salida digital	26	Activo en LOW; activación inmediata y liberación filtrada.
HC-SR04 TRIG	18	Pulso de disparo.
HC-SR04 ECHO	19	Divisor resistivo obligatorio para reducir la señal de 5 V a un nivel apto para 3,3 V.
SG90 señal	21	Alimentación externa de 5 V y tierra común.

Configuración:
BinConfig.h. Los valores reales de resistencias y las fotografías del montaje no están documentados en las fuentes; deben añadirse al anexo tras inspeccionar el circuito.
2.2 Esquema eléctrico funcional
ESP32GPIO 27GPIO 26GPIO 18GPIO 19GPIO 21GNDIR1 OUTIR2 OUTHC-SR04 TRIGHC-SR04 ECHO 5 VDivisor resistivoSG90 señalFuente 5 V adecuadaHC-SR04 VCCSG90 VCCTierra común: ESP32, sensoresy fuente servo

El diagrama representa conexiones funcionales y no sustituye un esquema con valores de resistencias, alimentación de cada módulo IR y fotografía del cableado real. Verificar que las salidas IR hacia el ESP32 no excedan 3,3 V.
2.3 Procesamiento local y calibración

IR1 se filtra durante 50 ms y genera una apertura ante un cambio a detección en modo automático. Una detección sostenida no debe generar nuevas solicitudes hasta liberarse. IR2 se activa inmediatamente y filtra la liberación; si se detecta durante el cierre, se ordena reapertura. El SG90 avanza en pasos configurados de 1° cada 20 ms, entre 0° y 90°. El tiempo inicial de espera abierta es 5 s, modificable desde el Shadow.

El HC-SR04 toma cinco muestras, separadas 70 ms, con timeout de 25 ms por eco; requiere al menos tres válidas y utiliza la mediana. La distancia se convierte mediante distancia_cm = duración_us × 0,0343 / 2. Con distancias configuradas de vacío 30 cm y lleno 5 cm:
llenado (%) = limitar(100 × (30 − distancia_cm) / (30 − 5), 0, 100)

Estas distancias son provisionales, no resultados de calibración. El ultrasonido se mide con la tapa estimada cerrada y el mecanismo detenido. Un resultado inválido no se sustituye por una medida inventada.
3. Diseño del sistema distribuido
3.1 Arquitectura
UsuarioSkill Alexa es-MXLambda basurero_alexaDynamoDB basurero_usuariosShadow clásico por ThingESP32 sensores y servoBroker AWS IoT CoreRule telemetríaRule eventosRule capacidadLambda almacenamientoDynamoDB basurero_historialLambda control capacidadMQTT/TLSdesired.capacityLock

Se documenta un Thing físico basurero-grupo3-01 y un Thing simulado basurero-grupo3-02. El segundo permite probar el enrutamiento en nube y no representa otro ESP32 físico.
3.2 Clases y responsabilidades
SmartBin+begin()+update()+setMode()+requestOpen()+requestClose()ProximitySensor+begin()+update()+detected()ObstacleSensor+begin()+update()+detected()UltrasonicSensor+start()+update()+distanceCm()+fillPercent()LidServo+open()+close()+update()WiFiManager+begin()+update()AWSIoTManager+publish()+subscribe()+update()ShadowManager+onMessage()+update()conectividad

Las Lambdas usan diseño funcional y no se representan como clases Python: basurero_alexa resuelve usuarios y Shadow; basurero_almacenamiento valida y persiste; basurero_control_capacidad decide el bloqueo. Sus fuentes desplegadas no están en el repositorio, por lo que no se afirma una estructura interna de clases.
3.3 Secuencias

Consulta por voz: se consulta el último dato reportado, no necesariamente una medición nueva.
ShadowDynamoDB usuariosLambda AlexaAlexaShadowDynamoDB usuariosLambda AlexaAlexaUsuarioConsultar nivelIntent y userIdGetItem(userId)thingNameGetThingShadowreported.fillPercentRespuesta con último valorNivel reportado o no disponibleUsuario

Comando de tapa:
ESP32ShadowDynamoDB usuariosLambda AlexaAlexaESP32ShadowDynamoDB usuariosLambda AlexaAlexaUsuarioAbrir tapaIntent y userIdResolver Thing e incrementar IDdesired.commandOrden solicitadaupdate/deltaComprobar capacidad y servoOrdenar movimiento si se admitereported y limpieza condicionalUsuario

La respuesta de Alexa confirma la solicitud, no que la tapa haya alcanzado su posición.

Almacenamiento y regla automática en nube:
ShadowLambda capacidadRule capacidadDynamoDB historialLambda almacenamientoRule telemetríaIoT CoreESP32ShadowLambda capacidadRule capacidadDynamoDB historialLambda almacenamientoRule telemetríaIoT CoreESP32par[Histórico][Capacidad]Publicar telemetríaCoincide tópicoCampos + Thing + tiempoValidar y PutItemCoincide tópico y filtroThing y porcentajeLeer estado y actualizar desired si procedeDelta de capacityLock

Reglas Edge:
SG90SmartBinIR1 e IR2SG90SmartBinIR1 e IR2alt[Zona libre][Obstáculo presente]Proximidad estableVerificar automático y bloqueoSolicitar apertura gradualEsperar retardoConsultar obstáculoIniciar cierre gradualObstáculo durante cierreOrdenar reaperturaMantener apertura
3.4 Estados, prioridad y ubicación de la lógica
CerradaAbriendoCerrandoAbierta;IR2libreIR1 en automático u ordenadmitidaIR2 detecta obstáculoÁngulo de aperturaordenadoRetardo automático ocierre solicitadoÁngulo de cierre ordenadoIR2 activo o modo manualsin cierre

El estado inicial depende de IR2: el firmware ordena posición abierta si detecta obstáculo al arrancar y cerrada en caso contrario. El diagrama representa el ciclo posterior, no una medición de posición física.
Regla	Lugar	Motivo y prioridad
Filtrado IR1, apertura, retardo y protección IR2.	Edge	Respuesta local y operación sin depender de la red; IR2 tiene prioridad sobre el cierre.
Impedir nuevas aperturas si capacityLock está activo y el llenado es inválido o alcanza el umbral.	Edge	Aplica la restricción disponible localmente; no bloquea una reapertura de seguridad por IR2.
Bloqueo/desbloqueo por umbral con histéresis.	Nube	Permite cambiar la regla y emplea telemetría y Shadow; no actúa mientras no lleguen datos.
Solicitud por voz.	Nube y Edge	Alexa modifica desired; el ESP32 evalúa sus restricciones locales antes de actuar. Manual deshabilita la apertura por IR1 y el cierre temporizado.

La Lambda de capacidad documentada bloquea al superar o igualar el umbral y libera al bajar al umbral menos 10 puntos; en la banda intermedia conserva el estado. Con umbral 90 %, los puntos son 90 % y 80 %. La fuente desplegada no está exportada para verificar aquí su implementación exacta.
3.5 Shadow y protocolo

El Shadow clásico separa desired (solicitud), reported (último estado informado) y delta (diferencia). El ESP32 solicita get al conectar y escucha respuestas y cambios.
Atributo	Tipo y valores	Sección y propósito
mode	String: AUTOMATICO, MANUAL	Desired y reported: modo.
autoCloseSeconds	Entero 1–3600	Desired y reported: retardo en segundos.
fillAlertThreshold	Entero 1–100 en firmware	Desired y reported: umbral de llenado. La Lambda de Alexa revisada acepta 11–100.
capacityLock	Boolean	Desired y reported: habilitación de la restricción local de capacidad.
lidState	cerrada, abriendo, abierta, cerrando, no_disponible	Reported: estado estimado.
fillPercent, distanceCm	Números: porcentaje 0–100 y cm	Reported: última medición válida. Sin medición se envía null, que elimina el atributo del Shadow.
irFrontDetected, irObstacleDetected	Boolean	Reported: sensores IR.
command	Objeto {id, action, origin}	Desired transitorio; acción abrir o cerrar, origen voz o nube.

El ID de comando es un entero positivo creciente por Thing. El ESP32 persiste el mayor ID en NVS antes de encolarlo y descarta los antiguos; después solicita retirar desired.command con la versión del Shadow. La política es como máximo una vez: una interrupción entre persistir el ID y procesar la cola puede perder la orden. Un update/accepted de limpieza no verifica movimiento físico.

Tópicos del Shadow: $aws/things/<thingName>/shadow/get, get/accepted, get/rejected, update, update/delta, update/accepted y update/rejected, bajo el mismo prefijo. Los tópicos propios son basurero/<thingName>/telemetria y basurero/<thingName>/eventos.
3.6 DynamoDB y patrones de acceso
Tabla	Claves documentadas	Contenido y acceso
basurero_usuarios	PK userId String; sin SK documentada.	thingName para resolver un usuario mediante GetItem. Filas especiales #seq#<thing> contienen commandSeq numérico.
basurero_historial	PK thingName String; SK timestampId String.	Elementos TELEMETRIA y EVENTO; consulta por Thing y, si el formato de SK lo permite, por rango temporal.

Los elementos pueden contener receivedAt, fillPercent, distanceCm, mode, lidState y, para eventos, tipo y origen. El firmware actual publica eventos con campos event, origin, detail y opcionalmente commandId; el formato efectivamente persistido debe contrastarse con la Lambda desplegada. No se han confirmado GSI ni TTL. No se debe suponer que exista historial de bloqueos solo porque cambie el Shadow.
3.7 Reglas IoT y funciones Lambda

Según el registro de configuración, se utilizaron las siguientes reglas:
-- basurero_regla_telemetria -> basurero_almacenamiento
SELECT *, topic(2) AS thingName, 'TELEMETRIA' AS recordType,
       timestamp() AS receivedAt
FROM 'basurero/+/telemetria'
-- basurero_regla_eventos -> basurero_almacenamiento
SELECT *, topic(2) AS thingName, 'EVENTO' AS recordType,
       timestamp() AS receivedAt
FROM 'basurero/+/eventos'
-- basurero_regla_capacidad -> basurero_control_capacidad
SELECT topic(2) AS thingName, fillPercent
FROM 'basurero/+/telemetria'
WHERE NOT isUndefined(fillPercent)

El filtro de la última regla solo comprueba presencia; la validación del valor corresponde a Lambda. Las dos reglas de almacenamiento comparten una función porque persiguen la misma responsabilidad. La capacidad está separada para aislar decisión, frecuencia y permisos.
Lambda	Disparador	Responsabilidad y recursos
basurero_alexa	Alexa Skills Kit	Resolver vínculo en basurero_usuarios, consultar o modificar Shadow y responder.
basurero_almacenamiento	Reglas de telemetría y eventos	Validar/enriquecer mensajes y escribir en basurero_historial. Reglas exactas de validación pendientes de exportación.
basurero_control_capacidad	Regla de capacidad	Leer Shadow y modificar desired.capacityLock cuando corresponda.
3.8 Diseño de interacción Alexa

Skill Basurero Inteligente, invocación «basurero inteligente», idioma es-MX. Un usuario queda vinculado a un Thing mediante código; la Lambda resuelve el userId en DynamoDB para cada operación.
Intent	Slot	Función y frase ilustrativa
VincularBasureroIntent	codigo	Vincular: «vincula mi basurero con el código …».
DesvincularBasureroIntent	—	Retirar asociación del usuario.
AbrirTapaIntent, CerrarTapaIntent	—	Solicitar apertura o cierre.
CambiarModoIntent	modo	«Cambia a modo manual».
CambiarUmbralIntent	porcentaje	Configurar umbral de llenado.
CambiarRetardoIntent	segundos	Configurar retardo de cierre.
ConsultarNivelIntent, ConsultarTapaIntent, ConsultarModoIntent	—	Consultar nivel, tapa o modo reportados.
ConsultarUmbralIntent, ConsultarRetardoIntent, ConsultarSensoresIntent, ConsultarBloqueoIntent	—	Consultar parámetros, sensores o bloqueo.

Las frases describen el diseño; el JSON actual del modelo de interacción no está exportado para verificar cada expresión exacta. El registro documenta una corrección de reconocimiento de ConsultarBloqueoIntent y una respuesta posterior correcta.
3.9 Seguridad y permisos

El cliente utiliza MQTT sobre TLS con certificado X.509, clave privada y CA de confianza. La plantilla
CloudSecrets.example.h no contiene secretos; CloudSecrets.h está excluido mediante .gitignore. Las claves privadas y credenciales WiFi no deben incluirse en el repositorio ni en capturas.

La
política IoT de ejemplo limita iot:Connect, Publish, Subscribe y Receive a los recursos necesarios para un Thing. El registro AWS indica que se amplió el rol de capacidad para permitir los Shadows de ambos Things. Permisos mínimos previstos:
Identidad	Acceso necesario
Certificado del ESP32	Conectar con su client ID y publicar/suscribir/recibir en sus tópicos de Shadow, telemetría y eventos.
Lambda Alexa	Operaciones de vínculo y contador en basurero_usuarios, DescribeThing, lectura y actualización de Shadows autorizados y logs.
Lambda almacenamiento	dynamodb:PutItem sobre historial y logs.
Lambda capacidad	Lectura y actualización del Shadow de los Things autorizados y logs.
IoT Rules y Alexa	Permiso de invocación de las Lambdas correspondientes.

El detalle completo de las políticas efectivamente desplegadas no está exportado; los permisos anteriores son los mínimos del diseño y deben contrastarse con IAM. Los códigos de vinculación estáticos del prototipo deben sustituirse por credenciales temporales para una aplicación real.
3.10 Mockups de reportes

Los siguientes son diseños, no pantallas implementadas ni datos reales.
+----------------------------------------------------------+
| REPORTE 1 — Evolución de llenado                         |
| Thing: [seleccionar]  Desde: [fecha]  Hasta: [fecha]      |
| Gráfico: porcentaje de llenado por hora                   |
| Última lectura: [ -- % ]    Máximo del período: [ -- % ] |
+----------------------------------------------------------+

Fuente prevista: Query sobre basurero_historial por thingName y rango de timestampId, filtrando telemetrías válidas y ordenando por tiempo. La consulta por rango depende de comprobar el formato real de la SK. Sirve para planificar el vaciado.
+----------------------------------------------------------+
| REPORTE 2 — Eventos de la tapa                          |
| Thing: [seleccionar]  Período: [seleccionar]             |
| Fecha/hora | Evento | Origen | ID de orden              |
| [ ... ]    | [ ...] | [ ...] | [ ... ]                  |
| Aperturas por origen: Edge [--] Voz [--] Nube [--]      |
+----------------------------------------------------------+

Fuente prevista: consulta por Thing y período, seguida de filtro y agregación de eventos apertura por origin. Una consulta adicional por cada PK permite comparar Things. DynamoDB FilterExpression no reduce la lectura de elementos ya evaluados; sin índice específico el filtrado puede aumentar el costo.
3.11 Especificación de mensajes de aplicación (ejemplos de diseño)

Los mensajes ilustran el contrato esperado y deben cotejarse con los payloads reales del firmware y de la Lambda antes de considerarse definitivos. La identidad del dispositivo se deriva del tópico y se valida en backend; no confiar en un thingName enviado libremente por el cliente.

Telemetría (ejemplo):
{
  "fillPercent": 35,
  "distanceCm": 16.2,
  "mode": "AUTOMATICO",
  "lidState": "cerrada",
  "irFrontDetected": false,
  "irObstacleDetected": false,
  "valid": true
}

Evento (ejemplo):
{
  "event": "apertura",
  "origin": "edge",
  "detail": "proximidad_ir1",
  "commandId": null
}
Mensaje	Emisor → destino	Frecuencia / disparador	Campos mínimos	Validación
Telemetría	ESP32 → tópico basurero/<thing>/telemetria	Periódico, documentado cada 30 s	Medición, estado, modo, sensores	Rangos, tipos, lectura válida y timestamp añadido por backend
Evento	ESP32 → tópico basurero/<thing>/eventos	Cambio/acción relevante	event, origin, detail, commandId si existe	Valores permitidos y tamaño de mensaje
Comando	Alexa/Lambda → Shadow desired → ESP32	Bajo solicitud	ID creciente, acción, origen	Autorización, ID no procesado, capacidad y seguridad IR2
Reported	ESP32 → Shadow	Al cambio y publicación periódica	Modo, sensores, nivel, estado estimado	No reportar posición física confirmada; inválidos no deben convertirse a cero
3.12 Contrato de Shadow y ejemplos de estados

desired se utiliza para solicitudes remotas y configuración; reported para lo que el firmware acepta/aplica o conoce. delta es calculado por AWS y no es un tercer espacio de escritura independiente. Los comandos son transitorios: procesar el identificador, informar resultado y limpiar el comando de manera versionada para evitar reejecución.

Ejemplo ilustrativo de configuración deseada:
{
  "state": {
    "desired": {
      "mode": "AUTOMATICO",
      "autoCloseSeconds": 5,
      "fillAlertThreshold": 90,
      "capacityLock": false,
      "command": {"id": 12, "action": "abrir", "origin": "voz"}
    }
  }
}

Ejemplo ilustrativo de estado reportado:
{
  "state": {
    "reported": {
      "mode": "AUTOMATICO",
      "autoCloseSeconds": 5,
      "fillAlertThreshold": 90,
      "capacityLock": false,
      "fillPercent": 35,
      "distanceCm": 16.2,
      "lidState": "cerrada",
      "irFrontDetected": false,
      "irObstacleDetected": false
    }
  }
}

Estos valores son ejemplos para explicar el formato, no una captura del Shadow real. Rangos y nombres finales deben coincidir con el firmware y el modelo de interacción.
3.13 Matriz de diseño AWS: Rules, Lambdas y recursos
Regla	Tópico / SQL	Acción	Validación y propósito
basurero_regla_telemetria	SELECT *, topic(2) AS thingName, 'TELEMETRIA' AS recordType, timestamp() AS receivedAt FROM 'basurero/+/telemetria'	Invocar basurero_almacenamiento	Enriquecer y registrar lecturas; Lambda valida payload y Thing
basurero_regla_eventos	SELECT *, topic(2) AS thingName, 'EVENTO' AS recordType, timestamp() AS receivedAt FROM 'basurero/+/eventos'	Invocar basurero_almacenamiento	Guardar eventos y origen, sin asumir que todos son entregados QoS 0
basurero_regla_capacidad	SELECT topic(2) AS thingName, fillPercent FROM 'basurero/+/telemetria' WHERE NOT isUndefined(fillPercent)	Invocar basurero_control_capacidad	Presencia de campo en SQL; tipo/rango y decisión se validan en Lambda
Lambda	Entrada	Operación/recursos	Salida esperada	Errores a manejar
basurero_almacenamiento	Payload de regla y thingName extraído del tópico	Validar; normalizar campos; escribir en basurero_historial	Item histórico con fecha, Thing, tipo y datos	Payload malformado, campo fuera de rango, permisos o error DynamoDB; registrar sin secretos
basurero_control_capacidad	Thing y fillPercent	Leer Shadow; evaluar threshold e histéresis; escribir desired.capacityLock solo si cambia	Solicitud de bloqueo/desbloqueo sincronizable con Edge	Lectura inválida, Thing no autorizado, Shadow inaccesible; evitar cambios repetidos
basurero_alexa	Intent, slots y userId Alexa	Resolver binding; leer Shadow o validar y actualizar desired; acceder a tabla de usuarios/contador y Shadow	Respuesta de voz que distingue solicitud enviada de estado reportado	Usuario sin vínculo, slot inválido, Thing inexistente, Shadow obsoleto, fallo de AWS

La tabla describe responsabilidades esperadas y el registro de configuración; no reemplaza el código desplegado. Exportar código o versiones sanitizadas de Lambda y revisar las entradas/salidas reales para cerrar trazabilidad.
3.14 DynamoDB: claves, tipos y patrones de acceso
Tabla	PK/SK	Tipos y campos principales	Accesos que satisface
basurero_usuarios	PK userId (String); no se documenta SK	thingName (String), commandSeq (Number) en registro de secuencia; timestamps/status si configurados	Obtener Thing vinculado por userId; obtener/incrementar secuencia por Thing; vincular/desvincular usuario
basurero_historial	PK thingName (String), SK timestampId (String)	recordType, receivedAt, fillPercent, distanceCm, mode, lidState, event, origin, detail, commandId según payload	Query por dispositivo; rango temporal si SK sortable; filtrar lecturas/eventos; contar/agregar en aplicación

No se han confirmado índices secundarios (GSI/LSI) ni TTL; se reportan como “no documentados/no configurados” hasta verificar la consola o IaC. La retención debe definirse explícitamente antes de habilitar TTL. Un FilterExpression filtra resultados después de lectura y no reduce automáticamente unidades consumidas.
3.15 Skill de Alexa: ejemplos de utterances
Intent	Slot y tipo	Frases de muestra	Respuesta/criterio
VincularBasureroIntent	codigo (código de vinculación)	“vincula mi basurero con el código {codigo}”	Confirmar vínculo solo si validación concluye
DesvincularBasureroIntent	—	“desvincula mi basurero”	Confirmar si se eliminó la asociación
AbrirTapaIntent / CerrarTapaIntent	—	“abre la tapa”; “cierra la tapa”	Confirmar solicitud, no movimiento físico
CambiarModoIntent	modo (AUTO/MANUAL)	“cambia a modo manual”	Repetir modo aceptado/reportado
CambiarUmbralIntent	porcentaje (1–100; Lambda revisada restringe 11–100)	“configura alerta al 85 por ciento”	Validar rango antes de escribir Shadow
CambiarRetardoIntent	segundos (1–3600)	“configura cierre en 5 segundos”	Confirmar valor aceptado
ConsultarNivelIntent	—	“qué tan lleno está”	Informar último dato y frescura
ConsultarTapaIntent	—	“la tapa está abierta”	Informar estado estimado
ConsultarModoIntent	—	“está en automático”	Informar modo reportado
ConsultarUmbralIntent / ConsultarRetardoIntent	—	“cuál es el umbral”; “cuánto tarda en cerrar”	Informar parámetros reportados
ConsultarSensoresIntent / ConsultarBloqueoIntent	—	“qué sensores están activos”; “está bloqueado por llenado”	Informar sensores y lock reportados

Seguridad del vínculo: el identificador de usuario de Alexa se usa para resolver un Thing asociado, pero la autorización de vinculación requiere validar un código de vinculación y su ciclo de vida. Documentar expiración, un solo uso, bloqueo de intentos y desvinculación si existen; si no existen, declarar esa limitación y no presentar el prototipo como autenticación de producción.
3.16 Reportes: indicador, fuente y consulta
Mockup	Indicador	Fuente/consulta	Decisión que apoya
Estado actual	Último nivel válido, timestamp, modo, bloqueo, tapa estimada	Shadow reported o último item del histórico por Thing	Saber si requiere vaciado y si dato está actualizado
Tendencia de llenado	Serie fillPercent por período	Query por PK thingName y SK temporal; excluir inválidos en aplicación	Estimar cuándo alcanzará el umbral
Eventos	Apertura/cierre, origen, fecha, commandId	Query por Thing y período; filtrar tipo EVENTO	Revisar uso y actividad
Resumen	Conteos por origen y alertas	Agregación de eventos descargados; si escala, diseñar índice/tabla agregada	Comparar automatización y control por voz

Los mockups son diseños de visualización. Las consultas deben ajustarse al formato real de la clave de ordenamiento y no se presentan como dashboards desplegados.
4. Desarrollo e implementación

El punto de entrada
main.cpp ejecuta el control local y crea una tarea independiente para red. Las colas intercambian instantáneas de estado, solicitudes y eventos.
SmartBin.cpp coordina sensores, servo, modos y prioridad de IR2;
LidServo.cpp mueve la tapa gradualmente.
UltrasonicSensor.cpp filtra muestras.
WiFiManager.cpp,
AWSIoTManager.cpp y
ShadowManager.cpp implementan conexión, TLS y sincronización. La lista de bibliotecas está en
platformio.ini.

La telemetría y el reported se programan cada 30 s; esta frecuencia reduce tráfico frente a publicar cada lectura y es adecuada como punto de partida para un llenado que cambia lentamente. La seguridad del cierre no espera a ese intervalo. MQTT usa QoS 0 y la cola de eventos en RAM es limitada; durante desconexiones prolongadas pueden perderse eventos. Tras reconectar se vuelve a consultar el Shadow. Los parámetros cloud están en RAM y tras un reinicio sin red se usan valores iniciales locales.

La configuración documentada incluye Thing físico, certificado, política IoT, tres reglas, tres Lambdas, dos tablas y skill descritos en el
registro AWS/Alexa. Esa fuente indica runtime Python 3.12 y región us-east-2. Deben exportarse las versiones finales de Lambdas, modelo de interacción y políticas para que el repositorio sea reproducible.
5. Pruebas y validaciones

El
plan T01–T20 describe procedimientos y criterios para sensores, tapa, modos, IR2, calibración, desconexión, MQTT, Shadow, reglas, DynamoDB, voz y separación de usuarios. Todos sus estados están marcados PENDIENTE; no se registran allí fechas, series de ensayos ni evidencias. Por ello, las observaciones siguientes se atribuyen al registro narrativo AWS y no se presentan como ensayos formales cerrados.
ID relacionado	Procedimiento descrito	Observación registrada	Alcance y evidencia
T11–T12	Conectar Thing físico y observar Shadow/monitor.	Se documentan MQTT conectado, Shadow recibido y reported aceptado.	
Registro §12–13; sin log adjunto ni métrica de estabilidad.
T18–T19	Consultar y ordenar tapa con Alexa.	Se documenta consulta y movimiento físico del servo; manual y automático se demostraron.	Registro narrativo; sin video o serie formal adjunta.
T17	Resolver dos identidades a dos Things.	Una cuenta Alexa real consultó Thing 01; una identidad simulada consultó Thing 02 con 35 %. Cambio de modo del Thing 02 sin modificar Thing 01.	No demuestra dos cuentas Alexa reales ni dos dispositivos físicos.
T20, T13–T14	Publicar 95 %, 85 % y 80 % en Thing simulado.	Se documentan bloqueo, permanencia y liberación de desired.capacityLock; las tres lecturas se visualizaron en historial.	Thing 02 simulado, sin confirmación física de reported por un segundo ESP32.
T10	Observar reconexión.	Se documenta una reconexión MQTT.	No equivale a una prueba controlada de operación offline y sincronización.
5.1 Protocolo para métricas pendientes

    Integridad y latencia: identificar N publicaciones válidas y contar registros únicos en DynamoDB; integridad = registros correspondientes/N × 100. Registrar duplicados, mensajes inválidos y pérdida. Registrar inicio y persistencia con relojes sincronizados para cada mensaje; comparar el máximo con 5 s. Con solo 20 mensajes, una pérdida implica 95 %: la resolución es insuficiente para caracterizar con precisión 99 %.
    Tiempo Edge y Cloud: registrar por separado detección o solicitud y primer movimiento físico observable, con reloj común o video de resolución declarada; repetir e informar máximo, valores individuales y condiciones.
    Calibración: medir distancias de referencia y diez lecturas en cada nivel; registrar porcentaje de referencia, valor calculado y diferencia en puntos porcentuales.
    Offline y reconexión: interrumpir la red, comprobar IR1, cierre e IR2; registrar duración de interrupción, eventos producidos, descartados y tiempo de recuperación, y comprobar aplicación del desired pendiente.
    Control e integridad: registrar prueba de comandos de voz, configuración reportada, obstáculos en ambos modos, rechazo de mensajes inválidos y aislamiento entre usuarios. Adjuntar capturas, logs o videos específicos sin secretos.

5.2 Matriz de pruebas resumida (20 pruebas)
ID	Prueba / procedimiento	Criterio de aceptación	Estado inicial
T01	IR1: acercar/retirar mano, 10 repeticiones	Detecta/libera en las 10 repeticiones; registrar falsos disparos	Pendiente de registro formal
T02	Apertura AUTO por IR1, repetir 10 veces	Abre solo si automático está activo y no hay bloqueo; detección sostenida no retriggera	Pendiente
T03	MANUAL: activar IR1 y comprobar comando manual	IR1 no abre automáticamente; comandos manuales se procesan según reglas	Pendiente
T04	Medir retardo de cierre en 5 ciclos	No inicia cierre antes del tiempo configurado; registrar duración real	Pendiente
T05	Activar IR2 antes del cierre	No inicia cierre mientras IR2 indique obstáculo	Pendiente
T06	Activar IR2 durante cierre, 5 intentos	Reacciona y ordena reapertura/acción segura según firmware; registrar tiempo	Pendiente
T07	Ejecutar 20 ciclos de tapa	Sin atascos, reinicios ni deformación; registrar anomalías	Pendiente
T08	Calibrar vacío/intermedio/alto, 10 muestras por punto	Error máximo objetivo ≤10 puntos porcentuales; adjuntar tabla de distancia/estimación	Pendiente
T09	Provocar eco inválido/fuera de rango	Lectura se marca inválida; no se almacena como nivel real ni se inventa valor	Pendiente
T10	Cortar Wi-Fi/Internet y probar funciones locales	IR1, IR2, temporizador y sensor local continúan según diseño; registrar pérdida de eventos	Pendiente
T11	Ver conexión TLS y publicar telemetría/evento	Conexión y mensajes visibles en tópico esperado	Pendiente formal; registro narrativo existente
T12	Cambiar Shadow desired y observar firmware/reported	Valor validado/aplicado; reported refleja estado; comando se limpia sin reejecución	Pendiente formal; registro narrativo existente
T13	Publicar telemetría/evento para activar Rule	Se invoca Lambda prevista y se registra log correlacionable	Pendiente formal
T14	Enviar 10 lecturas y 5 eventos válidos	Items aparecen en historial con Thing, timestamp y tipo correctos	Pendiente formal; registro narrativo parcial
T15	Enviar 5 payloads malformados o fuera de rango	Rechazados/registrados como inválidos; no contaminan datos válidos ni generan comando	Pendiente
T16	Enviar 20 mensajes con IDs únicos	Calcular integridad; objetivo ≥99% y latencia almacenamiento ≤5 s; informar número de muestra	Pendiente
T17	Verificar 2 bindings y usuario sin binding	Cada identidad accede solo al Thing vinculado; usuario no vinculado denegado	Pendiente formal; prueba simulada descrita
T18	Consultar nivel, tapa, modo y lectura ausente/desactualizada con Alexa	Respuesta usa datos reportados y distingue dato faltante o antiguo	Pendiente formal
T19	Comandos de voz: abrir, cerrar, modo, retardo y umbral; obstáculo y usuario no vinculado	Comando autorizado; Edge respeta IR2/capacidad; respuesta no afirma posición física confirmada	Pendiente formal; registro narrativo
T20	Enviar 95%, 85%, 80% y casos inválidos; repetir con nube desconectada	Con threshold 90%, bloqueo ≥90%, mantiene entre 80–89 y libera ≤80; Edge aplica último valor; invalidez no cambia lock	Pendiente formal; Thing simulado documentado
5.3 Registro de ejecución por prueba

Completar una fila por ejecución y conservar datos originales:
ID	Fecha/hora (UTC−4)	Responsable	Commit/versión	Condición y muestra	Resultado medido	Estado	Evidencia/archivo	Fallo y re-prueba
Txx	[Completar]	[Completar]	[Completar]	[Completar]	[Completar]	PENDIENTE	[Ruta]	[Completar]

Una prueba se marca APROBADA solo si se ejecutó, el resultado cumple su criterio y existe evidencia. Si no se ejecutó, debe permanecer PENDIENTE; si falla, se documenta causa, corrección y re-prueba sin borrar el fallo original.
5.4 Evidencia mínima por subsistema
Subsistema	Evidencia recomendada
Hardware/Edge	Fotos de montaje y cableado, divisor ECHO medido, video IR1/IR2/tapa, logs Serial, build y 20 ciclos
Sensor de llenado	CSV de distancias de referencia, muestras crudas, mediana, porcentaje y error
MQTT/Shadow	Capturas tópicos, desired/reported/delta, IDs de comando y confirmación de limpieza
Rules/Lambda	Export SQL/acciones, logs de invocación y payload sanitizado
DynamoDB	Items sanitizados, Query por Thing/rango, conteo de mensajes y errores
Alexa	Modelo de intents/slots exportado, Simulator, consultas y comandos, vínculo/desvínculo
Offline/recovery	Video/log antes, durante y después de corte; tiempo de reconexión; pérdidas
Seguridad	Política IoT, roles IAM y .gitignore revisados, sin claves, certificados privados ni datos personales
6. Resultados y análisis
Resultado documentado	Valor o alcance	Límite
Capacidad en Thing 02 simulado	95 % → bloqueo solicitado; 85 % → conserva; 80 % → desbloqueo solicitado.	Es un ensayo cloud, no mide respuesta física ni latencia.
Historial	Se visualizaron las tres telemetrías simuladas y elementos previos del Thing 01.	Sin conteo completo de enviados/recibidos ni medición de latencia.
Multi-Thing	Consulta simulada de Thing 02: 35 %; cambio de modo aislado respecto a Thing 01.	Una identidad es simulada y solo existe un equipo físico documentado.
Control físico y voz	El registro describe apertura/cierre del servo y comportamiento de modos.	La posición reportada es estimada; faltan evidencias individuales y tiempos controlados.

La separación Edge/nube mantiene la protección inmediata cerca del actuador y deja la regla de capacidad configurable en AWS. La desconexión limita la actualización de esa regla y la persistencia remota. El uso de QoS 0 y cola finita impide afirmar entrega completa de eventos. Los cambios rápidos de IR2 anotados en el registro justifican comprobar su estabilidad física y su zona de detección.
6.1 Estimación preliminar de tráfico y costo

Con operación continua durante 30 días y una publicación de telemetría cada 30 s:

    Telemetría: 30 × 24 × 60 × 60 / 30 = 86 400 publicaciones por dispositivo y mes.
    Reported: otras 86 400 solicitudes periódicas como aproximación, más actualizaciones por reconexión y comandos.
    Cada telemetría puede activar una invocación de almacenamiento y una de capacidad: hasta 86 400 de cada tipo, sujeto al filtro y a la operación efectiva. Cada evento adicional puede activar almacenamiento.
    Si cada telemetría válida genera un PutItem, se estiman 86 400 escrituras lógicas de telemetría, más eventos; lecturas y escrituras adicionales dependen de consultas, Alexa y lógica.

Estas cifras son aproximaciones de diseño, no facturación: tamaños de mensaje, respuestas del Shadow, redondeos de facturación, reintentos, persistencia real, solicitudes de Alexa, capacidad de DynamoDB, región y políticas comerciales alteran el costo. Falta consultar precios y límites vigentes de la cuenta y región y completar el importe mensual; no se afirma que esté dentro de una capa gratuita.
7. Conclusiones

    El firmware separa sensores, actuador y red; la protección IR2 se ejecuta en el Edge y no depende de una orden de la nube.
    El registro de configuración describe una demostración de integración con Alexa y un ensayo de histéresis 95/85/80 % en un Thing simulado. Estas observaciones respaldan los flujos descritos, pero no cuantifican confiabilidad, latencia ni desempeño físico del segundo Thing.
    El Shadow permite separar solicitud y estado reportado. En este prototipo el estado de la tapa sigue siendo una estimación, y la confirmación de una orden no demuestra que el mecanismo haya alcanzado una posición.
    La evidencia disponible no permite concluir que se alcanzan las metas de integridad, latencia, exactitud, operación offline o costo; esas verificaciones requieren registros adicionales.

8. Recomendaciones

    Calibrar vacío y lleno con mediciones reales del recipiente y repetir el ensayo de exactitud.
    Incorporar detección física de posición y protección mecánica adecuada para reducir incertidumbre y riesgos durante el cierre.
    Evaluar una estrategia de almacenamiento local y reenvío si se necesita garantizar el histórico durante interrupciones.
    Para producción, reemplazar códigos estáticos de vinculación por credenciales temporales y revisar permisos efectivos de todos los roles.
    Evaluar un mecanismo para detectar ausencia de telemetría mediante ejecución programada; una regla disparada por mensajes no detecta por sí misma que dejaron de llegar.
