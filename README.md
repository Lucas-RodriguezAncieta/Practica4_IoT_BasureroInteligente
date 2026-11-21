# Basurero inteligente — Práctica 4

Firmware Arduino para ESP32 con control local. La posición de la tapa se estima a partir de las órdenes enviadas; el SG90 no informa su posición real.

## Conexiones

| Elemento | Conexión |
| --- | --- |
| IR1, salida digital | GPIO 27 |
| IR2, salida digital | GPIO 26 |
| HC-SR04 TRIG | GPIO 18 |
| HC-SR04 ECHO | GPIO 19 **mediante divisor resistivo de 5 V a 3,3 V** |
| SG90, señal | GPIO 21 |

Alimenta el SG90 con una fuente de 5 V adecuada y conecta su GND al GND del ESP32 y de los sensores. Alimenta los sensores según las especificaciones de sus módulos y asegúrate de que sus salidas hacia el ESP32 no superen 3,3 V. El HC-SR04 se alimenta normalmente a 5 V. Los pines IR se configuran como entradas sin resistencias internas; usa módulos con salida digital correctamente polarizada.

## Ajustes

Edita [include/BinConfig.h](include/BinConfig.h). Los GPIO, polaridades activas, ángulos, intervalos, límites y calibración están centralizados allí. Los IR están configurados inicialmente como activos en LOW: cambia cada polaridad si el módulo lo requiere. El cierre y la apertura están inicialmente en 0° y 90°; ajústalos a la mecánica sin forzar el servo. El movimiento avanza un grado cada 20 ms.

**Calibración provisional:** `EMPTY_CM = 30` y `FULL_CM = 5` son valores iniciales, no medidas verificadas del recipiente. Mide desde el sensor hasta el fondo vacío y hasta el nivel máximo deseado, respectivamente; el valor de vacío debe ser mayor que el de lleno. El cálculo limita el resultado a 0–100 %. La lectura utiliza cinco intentos, descarta valores fuera del intervalo configurado o sin eco y requiere al menos tres válidos para calcular la mediana. El tiempo máximo de espera de eco es 25 ms por intento, por lo que el rango efectivo también está limitado por ese tiempo. La medición se realiza con la tapa estimada cerrada y el movimiento detenido. Se cancela si se abre la tapa. El último valor válido se conserva hasta que se complete otro ciclo; un ciclo inválido lo marca como no válido.

## Funcionamiento y comandos

El modo inicial es automático. Una transición filtrada de IR1 a detección solicita la apertura; debe dejar de detectar antes de volver a producir otra solicitud. Al arrancar con IR1 activo, primero debe liberarse. La tapa permanece abierta al menos cinco segundos en modo automático antes de intentar cerrar. IR2 impide el cierre y provoca reapertura si detecta un obstáculo durante el movimiento. La detección de IR2 se aplica inmediatamente y su liberación se filtra. En modo manual no se abre por IR1 ni se cierra por temporizador; un cierre solicitado queda pendiente mientras haya un obstáculo. Si se detecta uno durante el cierre, se reabre y se vuelve a intentar el cierre cuando se libere la zona.

El servo recibe una posición inicial al arrancar: abierta si IR2 está activo, cerrada si no lo está. Como no existe realimentación, ese movimiento inicial puede partir de una posición física desconocida. Despeja la zona antes de encender; IR2 no garantiza detectar todos los obstáculos y no sustituye una protección mecánica.

En el monitor serie a **115200 baudios**, envía comandos en minúsculas terminados en salto de línea:

- `auto`: modo automático.
- `manual`: modo manual.
- `abrir`: solicita apertura en cualquier modo.
- `cerrar`: solicita cierre protegido por IR2 en cualquier modo; si está abriendo, espera a que termine de abrir.
- `estado`: muestra el estado y la última medición válida disponible.

La salida también informa cambios de sensores y tapa, resultados de medición, advertencias y un resumen periódico. Las interfaces públicas de `SmartBin` permiten conectar en el futuro un adaptador de comandos y telemetría; no se incluye integración con AWS ni Alexa.

### Diagnóstico

Al iniciar, el firmware informa si el servo está asociado al GPIO 21. Una configuración de ángulos inválida y un fallo de asociación producen mensajes distintos. Si el servo no está disponible, se suspende la medición de llenado porque no se puede estimar que la tapa esté cerrada.

Cada intento ultrasónico muestra su duración y distancia si es válido. También se distingue ECHO en alto antes del disparo, ausencia de un pulso medible dentro del tiempo límite y distancia fuera del rango configurado. El resultado indica si faltan muestras válidas; una calibración con vacío menor o igual que lleno genera un mensaje específico. Un mensaje de ausencia de eco no identifica por sí solo si la causa es el cableado, la alimentación, el sensor, la superficie o la distancia. Antes de completar la primera medición, el estado puede indicar que aún no hay una medición válida.

## Verificación manual

1. Revisa las conexiones, el divisor de ECHO, tierra común y la polaridad de las salidas IR. Comprueba los ángulos y despeja la trayectoria de la tapa.
2. En PlatformIO ejecuta **Build**; la dependencia ESP32Servo se obtiene mediante `lib_deps`. Corrige cualquier error que muestre tu entorno.
3. Conecta el ESP32 y ejecuta **Upload**. Abre el monitor serie a 115200 y configura el envío de salto de línea.
4. Comprueba las lecturas de IR1 e IR2 y ajusta su polaridad si corresponde. Verifica que una detección sostenida de IR1 no genere aperturas repetidas.
5. Con la trayectoria despejada y supervisión directa, comprueba apertura, espera y cierre; verifica que IR2 impida el cierre y que, al activarlo durante el cierre, se ordene la reapertura. Repite la protección en modo manual.
6. Mide y configura las distancias realistas de vacío y lleno. Comprueba la distancia y porcentaje con niveles conocidos, y verifica el aviso al desconectar o bloquear el eco.

No se ha compilado ni probado esta implementación.

## Conexión AWS IoT Core

La integración utiliza TLS en el puerto 8883, autenticación mutua X.509, el Shadow clásico y MQTT con PubSubClient 2.8 y ArduinoJson 6.21.5. No utiliza `setInsecure()`. El código de red corre en una tarea separada y se comunica con el control local mediante colas; si faltan WiFi o AWS, el control local sigue funcionando. La tarea de red puede esperar durante conexiones y operaciones MQTT, sin detener deliberadamente la tarea local. Los eventos usan QoS 0 y una cola en RAM de 16 elementos: no se garantiza su entrega, y los eventos adicionales se descartan si se llena la cola.

1. Copia `include/CloudSecrets.example.h` a `include/CloudSecrets.h`. Completa SSID, contraseña, endpoint AWS IoT **ATS** sin protocolo ni puerto, nombre exacto del Thing, CA raíz de Amazon adecuada, certificado X.509 activo y clave privada correspondiente. Conserva los delimitadores PEM y saltos de línea. No compartas este archivo: está en `.gitignore`. Los secretos quedan incluidos en el firmware grabado y deben protegerse también en los artefactos de compilación y la placa.
2. En AWS IoT Core configura el Thing, asocia el certificado activo y una política. Usa `aws-iot-policy.example.json` como punto de partida: sustituye REGION, ACCOUNT_ID y THING_NAME. Restringe el acceso a ese dispositivo. No se crean recursos automáticamente.
3. El dispositivo necesita acceso WiFi a Internet, DNS, NTP UDP/123 para obtener hora y MQTT TLS TCP/8883 hacia el endpoint. Se verifica el certificado del servidor con la CA proporcionada. El endpoint y el certificado deben corresponder a tu cuenta y región.
4. Compila y carga manualmente desde PlatformIO. En el monitor serie a 115200 comprueba conexión WiFi e IP, sincronización horaria, conexión MQTT, solicitudes de suscripción y respuesta al get del Shadow. Una suscripción «solicitada» confirma el envío, no una autorización SUBACK verificada; si no llegan respuestas revisa la política.
5. En AWS IoT Core, MQTT test client, suscríbete a `basurero/THING_NAME/telemetria`, `basurero/THING_NAME/eventos` y `$aws/things/THING_NAME/shadow/update/accepted`; sustituye THING_NAME. Consulta el Shadow clásico del Thing. Si inicialmente no existe, un get rechazado con 404 provoca que el dispositivo publique reported para crearlo.
6. Publica manualmente en `$aws/things/THING_NAME/shadow/update` un cambio de parámetros como:

```json
{"state":{"desired":{"mode":"MANUAL","autoCloseSeconds":5,"fillAlertThreshold":90,"capacityLock":false}}}
```

Comprueba el delta y el estado local antes de enviar una orden puntual. Con la tapa despejada, un ejemplo es:

```json
{"state":{"desired":{"command":{"id":1,"action":"abrir","origin":"nube"}}}}
```

Para la siguiente orden utiliza un entero `id` estrictamente mayor (1–4294967295), `action` igual a `abrir` o `cerrar`, y `origin` igual a `nube` o `voz`. El productor debe asignar identificadores únicos y crecientes, incluso después de reinicios; `voz` es una etiqueta proporcionada por el productor, no una prueba de procedencia. El ESP32 guarda en NVS el mayor ID antes de encolar la orden y descarta IDs menores o iguales. Envía una actualización condicional por versión que borra `desired.command`; espera `update/accepted`, y ante conflictos o pérdida de respuesta vuelve a consultar. Una actualización concurrente no debe borrarse con una versión anterior.

La deduplicación es **como máximo una vez**, no garantiza ejecución: un reinicio después de guardar el ID y antes de procesar la cola puede perder la orden. La confirmación de limpieza solo confirma el registro y retiro del comando del Shadow, no un movimiento físico. Un evento `orden` indica aceptación o rechazo local; `apertura` y `cierre` indican que comenzó el movimiento **estimado**, sin sensor de posición ni confirmación física. Si necesitas comprobar el resultado físico o garantizar la ejecución, se requiere hardware de realimentación y un protocolo adicional.

Los valores admitidos de `autoCloseSeconds` son 1–3600 y de `fillAlertThreshold` 1–100. Por defecto son 5 s y 90 %. `capacityLock=true` impide nuevas aperturas solicitadas si el llenado válido es mayor o igual al umbral, o si no hay lectura válida; no impide la reapertura de seguridad ante IR2. El ajuste es temporal en RAM y se recupera del Shadow al reconectar; durante un reinicio sin nube se usan los valores iniciales locales. La protección IR2 sigue activa en ambos modos.

El dispositivo publica `reported` cada 30 s y tras la sincronización inicial, y telemetría cada 30 s. Sin una medición válida, la telemetría usa `null`; en el Shadow, AWS elimina los atributos `reported` enviados como `null`. El buffer MQTT es de 8192 bytes y debe incluir tópico y carga útil; la capacidad de análisis JSON del Shadow es de 6144 bytes. Mantén el documento por debajo de esos límites, incluida la metadata que AWS añade. Mensajes mayores se descartan y deben reducirse. Los valores de configuración rechazados permanecen en desired para que puedan corregirse.

Tópicos utilizados: `$aws/things/THING_NAME/shadow/get`, `get/accepted`, `get/rejected`, `update`, `update/delta`, `update/accepted` y `update/rejected` (los sufijos pertenecen al mismo prefijo `$aws/things/THING_NAME/shadow/`); además, `basurero/THING_NAME/telemetria` y `basurero/THING_NAME/eventos`.

Para comprobar tolerancia a desconexiones, interrumpe WiFi manualmente y verifica IR, cierre y protección de obstáculos; restablece la red y comprueba reconexión, get y reported. No se ha compilado ni probado esta integración.
