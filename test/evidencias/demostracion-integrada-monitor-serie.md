# Demostración integrada — registro serie

El equipo informó que realizó satisfactoriamente la demostración física y que los videos se encuentran en la [carpeta de evidencias en Google Drive](https://drive.google.com/drive/folders/1e1kBgZhMyD00joOQXWiHsFJz2qgqlJlf?usp=drive_link). La carpeta y los videos no fueron reproducidos en esta revisión; debe comprobarse que el docente pueda acceder.

La salida serie proporcionada no incluye marcas de tiempo para cada línea. Los estados de la tapa son estimaciones del firmware, y la observación del movimiento físico corresponde al equipo.

## IR2 antes y durante el cierre

```text
IR2: obstaculo
Modo: AUTOMATICO | IR1: libre | IR2: obstaculo | Tapa estimada: abierta
IR2: libre
Tapa (estimada): cerrando
Tapa (estimada): cerrada
...
Tapa (estimada): cerrando
IR2: obstaculo
Obstaculo durante cierre: reabriendo.
Tapa (estimada): abriendo
Tapa (estimada): abierta
```

El registro también contiene una segunda detección de obstáculo durante el cierre y la orden de reapertura. El equipo confirmó el funcionamiento físico de la demostración.

## Shadow y órdenes de apertura

```text
Shadow delta recibido.
Comando 1795244259 registrado para procesamiento local.
Comando 1795244259 procesado: abrir solicitado.
Tapa (estimada): abriendo
Shadow: solicitada limpieza condicional del comando.
Shadow: limpieza de comando confirmada.
Tapa (estimada): abierta
...
Shadow delta recibido.
Comando 1795244260 registrado para procesamiento local.
Comando 1795244260 procesado: abrir solicitado.
```

La salida muestra el procesamiento de órdenes y la confirmación de limpieza de un comando; no identifica por sí sola el origen de cada orden ni la duración del recorrido de voz.

## Desconexión y recuperación

```text
WiFi desconectado.
Intentando conectar WiFi...
[ssl_client.cpp] ... (-76) UNKNOWN ERROR CODE (004C)
MQTT desconectado; reintentando.
...
WiFi conectado.
Conectando MQTT TLS a AWS IoT...
MQTT conectado.
Shadow: solicitud get enviada.
Shadow recibido.
Comando 1795244261 registrado para procesamiento local.
Comando 1795244261 procesado: abrir solicitado.
Tapa (estimada): abriendo
Shadow: limpieza de comando confirmada.
Shadow: reported enviado.
Shadow: reported aceptado.
```

El mensaje de error TLS apareció durante la interrupción; la salida posterior registra recuperación de WiFi y MQTT. El registro muestra una orden procesada tras la reconexión, pero no indica cuándo se escribió en `desired` ni cuantifica el tiempo de recuperación. El equipo informa que las funciones locales también se observaron durante la demostración sin WiFi; esa observación debe contrastarse con el video.

## Distancia de ejemplo

El monitor registró cinco muestras mostradas como 12,9 cm y un llenado estimado de 68,5 %. Esta lectura demuestra que el firmware produjo una estimación durante la demostración; no constituye por sí sola una calibración contra una referencia física.
