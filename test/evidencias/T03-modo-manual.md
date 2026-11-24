# T03 — Modo manual

## Procedimiento y observación del equipo

El equipo activó el modo manual, acercó la mano a IR1 y probó los comandos manuales de apertura y cierre. Confirmó que:

- IR1 detectó la mano y la tapa permaneció cerrada.
- La tapa abrió con el comando `abrir`.
- La tapa permaneció abierta sin cerrar automáticamente.
- La tapa cerró con el comando `cerrar`.

El equipo informó que dispone de fotografías del funcionamiento físico, identificadas como `PruebaT02_1` a `PruebaT02_3`. Esos nombres fueron proporcionados por el equipo para esta evidencia de T03; no se comprobó el contenido ni la ruta exacta de los archivos durante este registro.

## Extracto del monitor serie

```text
IR1: detectado
Modo: MANUAL | IR1: detectado | IR2: libre | Tapa estimada: cerrada
IR1: libre
...
Apertura rechazada: bloqueo de capacidad activo.
...
Tapa (estimada): abriendo
Tapa (estimada): abierta
Modo: MANUAL | IR1: libre | IR2: libre | Tapa estimada: abierta
Tapa (estimada): cerrando
Tapa (estimada): cerrada
```

El registro contiene también mensajes `Comando desconocido`; el texto recibido en esos casos no se incluyó en la salida, por lo que no se determina su causa. La apertura fue rechazada en una ocasión por el bloqueo de capacidad activo y posteriormente se registró una secuencia de apertura y cierre. El monitor informa estados estimados; la observación del movimiento físico fue comunicada por el equipo.

**Resultado:** aprobado para el procedimiento de T03 según las observaciones del equipo y el registro aportado. Esta prueba no sustituye una comprobación independiente de IR2.
