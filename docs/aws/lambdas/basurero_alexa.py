import json
import os
import re
import time
import unicodedata

import boto3
from botocore.exceptions import ClientError

TABLE_NAME = os.environ.get("TABLE_NAME", "basurero_usuarios")
AWS_ENDPOINT = os.environ.get(
    "IOT_ENDPOINT",
    "https://a1z8p2mejgol72-ats.iot.us-east-2.amazonaws.com"
)
SKILL_ID = os.environ.get("SKILL_ID", "")

users = boto3.resource("dynamodb").Table(TABLE_NAME)
iot = boto3.client("iot", region_name="us-east-2")
shadow_api = boto3.client(
    "iot-data",
    region_name="us-east-2",
    endpoint_url=AWS_ENDPOINT
)


def speak(message):
    return {
        "version": "1.0",
        "response": {
            "outputSpeech": {
                "type": "PlainText",
                "text": message
            },
            "shouldEndSession": True
        }
    }


def get_user_id(event):
    user = (
        (event.get("session") or {}).get("user")
        or ((event.get("context") or {}).get("System") or {}).get("user")
        or {}
    )
    return user.get("userId")


def slot_value(intent, key):
    slot = (intent.get("slots") or {}).get(key) or {}
    return (slot.get("value") or "").strip()


def normalize(text):
    text = unicodedata.normalize("NFKD", text)
    return "".join(
        ch for ch in text
        if not unicodedata.combining(ch)
    ).upper()


def get_linked_thing(user_id):
    result = users.get_item(Key={"userId": user_id})
    return (result.get("Item") or {}).get("thingName")


def pair_thing(user_id, intent):
    code = re.sub(r"\s+", "", slot_value(intent, "codigo"))

    try:
        pairing = json.loads(
            os.environ.get("PAIRING_CODES_JSON", "{}")
        )
    except ValueError:
        raise RuntimeError("Configuracion de vinculacion invalida")

    thing = pairing.get(code) if isinstance(pairing, dict) else None

    if not isinstance(thing, str) or not re.fullmatch(
        r"basurero-[A-Za-z0-9_-]{1,64}", thing
    ):
        return speak("El codigo de vinculacion no es valido.")

    try:
        iot.describe_thing(thingName=thing)
    except ClientError as error:
        if error.response["Error"]["Code"] == "ResourceNotFoundException":
            return speak("Ese basurero no existe en AWS.")
        raise

    users.put_item(Item={
        "userId": user_id,
        "thingName": thing
    })

    return speak("Tu basurero quedo vinculado correctamente.")


def read_reported(thing):
    try:
        response = shadow_api.get_thing_shadow(thingName=thing)
        document = json.loads(response["payload"].read())
        return (
            ((document.get("state") or {}).get("reported"))
            or {}
        )
    except ClientError as error:
        if error.response["Error"]["Code"] == "ResourceNotFoundException":
            return {}
        raise


def update_desired(thing, changes):
    payload = json.dumps({
        "state": {"desired": changes}
    }).encode("utf-8")

    shadow_api.update_thing_shadow(
        thingName=thing,
        payload=payload
    )


def next_command_id(thing):
    # Contador persistente por Thing.
    response = users.update_item(
        Key={"userId": "#seq#" + thing},
        UpdateExpression=(
            "SET commandSeq = "
            "if_not_exists(commandSeq, :base) + :step"
        ),
        ExpressionAttributeValues={
            ":base": int(time.time()),
            ":step": 1
        },
        ReturnValues="UPDATED_NEW"
    )

    value = int(response["Attributes"]["commandSeq"])

    if not 0 < value < 4294967296:
        raise RuntimeError("Identificador fuera de rango")

    return value


def lambda_handler(event, context):
    try:
        request = event.get("request") or {}
        kind = request.get("type")

        system = (
            (event.get("context") or {}).get("System")
            or {}
        )

        app_id = (
            (system.get("application") or {}).get("applicationId")
        )

        if SKILL_ID and app_id != SKILL_ID:
            return speak("Solicitud de otra habilidad.")

        if kind == "LaunchRequest":
            return speak(
                "Bienvenido a basurero inteligente. "
                "Puedes consultar el nivel, abrir la tapa "
                "o cambiar el modo."
            )

        if kind == "SessionEndedRequest":
            return {
                "version": "1.0",
                "response": {}
            }

        if kind != "IntentRequest":
            return speak("No pude reconocer esa solicitud.")

        intent = request.get("intent") or {}
        name = intent.get("name", "")

        if name in ("AMAZON.StopIntent", "AMAZON.CancelIntent"):
            return speak("Hasta pronto.")

        if name == "AMAZON.HelpIntent":
            return speak(
                "Puedes vincular tu basurero con un codigo, "
                "consultar su llenado, abrir la tapa "
                "o cambiar el modo."
            )

        user_id = get_user_id(event)

        if not isinstance(user_id, str) or not user_id:
            return speak("No pude identificar al usuario de Alexa.")

        if name == "VincularBasureroIntent":
            return pair_thing(user_id, intent)

        if name == "DesvincularBasureroIntent":
            users.delete_item(Key={"userId": user_id})
            return speak("La vinculacion se elimino.")

        thing = get_linked_thing(user_id)

        if not thing:
            return speak(
                "Primero debes vincular tu basurero "
                "con el codigo correspondiente."
            )

        # Comandos para el servomotor
        if name in ("AbrirTapaIntent", "CerrarTapaIntent"):
            action = (
                "abrir" if name == "AbrirTapaIntent"
                else "cerrar"
            )

            command_id = next_command_id(thing)

            update_desired(thing, {
                "command": {
                    "id": command_id,
                    "action": action,
                    "origin": "voz"
                }
            })

            return speak(
                f"He enviado la orden de {action} la tapa. "
                "El dispositivo confirmara su estado "
                "cuando la procese."
            )

        # Configuracion del modo
        if name == "CambiarModoIntent":
            mode = normalize(slot_value(intent, "modo"))

            if mode not in ("AUTOMATICO", "MANUAL"):
                return speak("Indica modo automatico o manual.")

            update_desired(thing, {"mode": mode})
            return speak(f"He solicitado el modo {mode.lower()}.")

        # Configuracion del umbral
        if name == "CambiarUmbralIntent":
            raw = slot_value(intent, "porcentaje")

            if not re.fullmatch(r"\d{1,3}", raw) or not 11 <= int(raw) <= 100:
                return speak(
                    "El umbral debe estar entre "
                    "once y cien por ciento."
                )

            update_desired(thing, {
                "fillAlertThreshold": int(raw)
            })

            return speak(
                f"He solicitado un umbral de {raw} por ciento."
            )

        # Configuracion del tiempo de cierre
        if name == "CambiarRetardoIntent":
            raw = slot_value(intent, "segundos")

            if not re.fullmatch(r"\d{1,4}", raw) or not 1 <= int(raw) <= 3600:
                return speak(
                    "El retardo debe estar entre uno "
                    "y tres mil seiscientos segundos."
                )

            update_desired(thing, {
                "autoCloseSeconds": int(raw)
            })

            return speak(
                f"He solicitado un tiempo de cierre "
                f"de {raw} segundos."
            )

        # Consultas: siempre utilizan reported
        reported = read_reported(thing)

        if not reported:
            return speak(
                "El basurero todavia no ha reportado su estado."
            )

        if name == "ConsultarNivelIntent":
            level = reported.get("fillPercent")

            if type(level) in (int, float):
                return speak(
                    f"El ultimo nivel reportado es "
                    f"{round(level)} por ciento."
                )

            return speak(
                "No hay una medicion de llenado valida."
            )


        if name == "ConsultarTapaIntent":
            state = reported.get("lidState")

            if not isinstance(state, str) or state in (
                "", "no_disponible", "sin estado"
            ):
                return speak(
                    "Todavia no tengo informacion confirmada "
                    "sobre el estado de la tapa."
                )

            return speak(f"La tapa figura como {state}.")


        if name == "ConsultarBloqueoIntent":
            bloqueo = reported.get("capacityLock")

            if not isinstance(bloqueo, bool):
                return speak(
                    "Todavia no tengo informacion confirmada "
                    "sobre el bloqueo del basurero."
                )

            if bloqueo:
                return speak(
                    "El basurero esta bloqueado por capacidad."
                )

            return speak(
                "El basurero no esta bloqueado por capacidad."
            )


        if name == "ConsultarModoIntent":
            mode = reported.get("mode")

            if mode not in ("AUTOMATICO", "MANUAL"):
                return speak(
                    "Todavia no tengo un modo confirmado "
                    "por el dispositivo."
                )

            return speak(
                f"El modo reportado es {mode.lower()}."
            )

        if name == "ConsultarUmbralIntent":
            threshold = reported.get("fillAlertThreshold")

            if (
                type(threshold) not in (int, float)
                or not 11 <= threshold <= 100
            ):
                return speak(
                    "Todavia no tengo un umbral de llenado "
                    "confirmado por el dispositivo."
                )

            return speak(
                f"El umbral reportado es "
                f"{threshold:g} por ciento."
            )

        if name == "ConsultarRetardoIntent":
            seconds = reported.get("autoCloseSeconds")

            if (
                type(seconds) not in (int, float)
                or not 1 <= seconds <= 3600
            ):
                return speak(
                    "Todavia no tengo informacion sobre "
                    "el retardo de cierre del dispositivo."
                )

            return speak(
                f"El retardo de cierre reportado "
                f"es {seconds:g} segundos."
            )

        if name == "ConsultarSensoresIntent":
            front_value = reported.get("irFrontDetected")
            obstacle_value = reported.get("irObstacleDetected")

            if (
                not isinstance(front_value, bool)
                and not isinstance(obstacle_value, bool)
            ):
                return speak(
                    "Todavia no tengo informacion de los sensores. "
                    "El dispositivo aun no ha reportado sus estados."
                )

            if front_value is True:
                front = "detecta presencia"
            elif front_value is False:
                front = "no detecta presencia"
            else:
                front = "todavia no ha reportado su estado"

            if obstacle_value is True:
                obstacle = "detecta un obstaculo"
            elif obstacle_value is False:
                obstacle = "no detecta obstaculos"
            else:
                obstacle = "todavia no ha reportado su estado"

            return speak(
                f"El sensor frontal {front}, "
                f"y el sensor de seguridad {obstacle}."
            )


        return speak("No tengo programada esa consulta.")

    except ClientError as error:
        print(
            "Error AWS:",
            error.response["Error"].get("Code")
        )
        return speak(
            "No pude comunicarme con AWS. "
            "Intentalo de nuevo."
        )

    except Exception as error:
        print(
            "Error procesando solicitud:",
            type(error).__name__
        )
        return speak(
            "Ocurrio un error al procesar la solicitud."
        )
