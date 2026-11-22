import json
import math
import re
import boto3

from botocore.exceptions import ClientError

# Configuracion AWS IoT Core
AWS_ENDPOINT = (
    "https://a1z8p2mejgol72-ats.iot.us-east-2.amazonaws.com"
)

DEFAULT_THRESHOLD = 90
HYSTERESIS = 10

iot = boto3.client(
    "iot-data",
    region_name="us-east-2",
    endpoint_url=AWS_ENDPOINT
)


def obtener_shadow(thing_name):
    try:
        response = iot.get_thing_shadow(
            thingName=thing_name
        )
        return json.loads(response["payload"].read())

    except ClientError as error:
        codigo = error.response["Error"]["Code"]

        if codigo == "ResourceNotFoundException":
            # El Shadow aun no tiene documento.
            return {}

        raise


def obtener_umbral(shadow):
    state = shadow.get("state", {})

    reported = state.get("reported") or {}
    desired = state.get("desired") or {}

    # Preferir el umbral aplicado por el dispositivo.
    umbral = reported.get(
        "fillAlertThreshold",
        desired.get("fillAlertThreshold", DEFAULT_THRESHOLD)
    )

    if type(umbral) is not int or not 1 <= umbral <= 100:
        return DEFAULT_THRESHOLD

    return umbral


def lambda_handler(event, context):
    if not isinstance(event, dict):
        return {
            "statusCode": 400,
            "message": "Evento invalido"
        }

    thing_name = event.get("thingName")
    llenado = event.get("fillPercent")

    if not isinstance(thing_name, str) or not re.fullmatch(
        r"basurero-[A-Za-z0-9_-]{1,64}", thing_name
    ):
        return {
            "statusCode": 400,
            "message": "Thing invalido"
        }

    if (
        type(llenado) not in (int, float)
        or not math.isfinite(llenado)
        or not 0 <= llenado <= 100
    ):
        return {
            "statusCode": 400,
            "message": "Medicion invalida"
        }

    # Reintentar ante cambios concurrentes del Shadow.
    for intento in range(2):
        shadow = obtener_shadow(thing_name)

        state = shadow.get("state", {})
        desired = state.get("desired") or {}

        umbral = obtener_umbral(shadow)
        liberacion = max(0, umbral - HYSTERESIS)

        print(
            f"Thing={thing_name}, llenado={llenado}, "
            f"bloqueo={umbral}, liberacion={liberacion}"
        )

        if llenado >= umbral:
            nuevo_bloqueo = True

        elif llenado <= liberacion:
            nuevo_bloqueo = False

        else:
            return {
                "statusCode": 200,
                "message": "Sin cambios: zona de histeresis"
            }

        bloqueo_actual = desired.get("capacityLock")

        if bloqueo_actual is nuevo_bloqueo:
            return {
                "statusCode": 200,
                "message": "El bloqueo ya tiene ese valor"
            }

        nuevo_estado = {
            "state": {
                "desired": {
                    "capacityLock": nuevo_bloqueo
                }
            }
        }

        # Evitar sobrescribir una version mas reciente.
        if "version" in shadow:
            nuevo_estado["version"] = shadow["version"]

        try:
            iot.update_thing_shadow(
                thingName=thing_name,
                payload=json.dumps(nuevo_estado).encode("utf-8")
            )

            print(f"capacityLock actualizado: {nuevo_bloqueo}")

            return {
                "statusCode": 200,
                "message": "Shadow actualizado",
                "capacityLock": nuevo_bloqueo,
                "fillPercent": llenado,
                "threshold": umbral
            }

        except ClientError as error:
            codigo = error.response["Error"]["Code"]

            if codigo == "ConflictException" and intento == 0:
                continue

            raise

    raise RuntimeError("No se pudo actualizar el Shadow")
