# T01 — Extracto del monitor serie

Extracto proporcionado por el equipo durante la prueba de proximidad. Los estados de la tapa son **estimados por el firmware**; la observación física del servo se complementa con fotografías y video.

```text
MQTT conectado.
Shadow: solicitud get enviada.
Shadow recibido.
Modo: AUTOMATICO
Shadow: reported enviado.
Shadow: reported aceptado.
IR1: detectado
Tapa (estimada): abriendo
IR1: libre
Tapa (estimada): abierta
Tapa (estimada): cerrando
Tapa (estimada): cerrada
```

El registro completo aportado contiene 11 secuencias de detección de IR1 seguidas de apertura y cierre estimados. Este extracto muestra una de ellas; los mensajes por sí solos no confirman la posición física ni permiten asegurar cuántos intentos físicos cumplieron el criterio de aceptación. El equipo informó que dispone de dos imágenes en `test` y de un video. Quedan pendientes el número de intentos físicos correctos y los enlaces a esas evidencias.
