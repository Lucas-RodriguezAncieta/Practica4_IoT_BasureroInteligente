
import boto3
import hashlib
import json
import re

from datetime import datetime, timezone
from decimal import Decimal
from botocore.exceptions import ClientError

TABLE_NAME = "basurero_historial"

dynamodb = boto3.resource("dynamodb")
table = dynamodb.Table(TABLE_NAME)


def validar_numero(valor, nombre, minimo, maximo):
    if isinstance(valor, bool) or not isinstance(valor, (int, float)):
        raise ValueError(f"{nombre}: tipo invalido")

    numero = Decimal(str(valor))

    if not numero.is_finite() or not (minimo <= numero <= maximo):
        raise ValueError(f"{nombre}: fuera de rango")

    return numero


def validar_texto(valor, nombre, permitidos=None, maximo=32):
    if not isinstance(valor, str) or not (1 <= len(valor) <= maximo):
        raise ValueError(f"{nombre}: texto invalido")

    if permitidos is not None and valor not in permitidos:
        raise ValueError(f"{nombre}: valor no permitido")

    return valor


def lambda_handler(event, context):
    try:
        if not isinstance(event, dict):
            raise ValueError("El mensaje debe ser un objeto JSON")

        thing = event.get("thingName")
        tipo = event.get("recordType")
        recibido = event.get("receivedAt")

        if not isinstance(thing, str) or not re.fullmatch(
            r"[A-Za-z0-9:_-]{1,128}", thing
        ):
            raise ValueError("thingName invalido")

        if tipo not in ("TELEMETRIA", "EVENTO"):
            raise ValueError("recordType invalido")

        if isinstance(recibido, bool) or not isinstance(recibido, int) or recibido <= 0:
            raise ValueError("receivedAt invalido")

        fecha = datetime.fromtimestamp(
            recibido / 1000, tz=timezone.utc
        ).isoformat(timespec="milliseconds").replace("+00:00", "Z")

        item = {
            "thingName": thing,
            "recordType": tipo,
            "receivedAt": recibido
        }

        if tipo == "TELEMETRIA":
            item["fillPercent"] = validar_numero(
                event.get("fillPercent"), "fillPercent", 0, 100
            )
            item["distanceCm"] = validar_numero(
                event.get("distanceCm"), "distanceCm", 2, 400
            )
            item["lidState"] = validar_texto(
                event.get("lidState"),
                "lidState",
                ("cerrada", "abriendo", "abierta",
                 "cerrando", "no_disponible")
            )
            item["mode"] = validar_texto(
                event.get("mode"),
                "mode",
                ("AUTOMATICO", "MANUAL")
            )

        else:
            item["event"] = validar_texto(
                event.get("event"), "event", maximo=24
            )
            item["origin"] = validar_texto(
                event.get("origin"),
                "origin",
                ("edge", "voz", "nube")
            )
            item["detail"] = validar_texto(
                event.get("detail"), "detail", maximo=32
            )

            if "commandId" in event:
                item["commandId"] = validar_texto(
                    event["commandId"], "commandId", maximo=64
                )

        device_ts = event.get("timestamp")
        if type(device_ts) is int and device_ts > 0:
            item["deviceTimestamp"] = device_ts

        huella = hashlib.sha256(
            json.dumps(event, sort_keys=True).encode("utf-8")
        ).hexdigest()[:16]

        item["timestampId"] = f"{fecha}#{tipo}#{huella}"

        try:
            table.put_item(
                Item=item,
                ConditionExpression="attribute_not_exists(thingName)"
            )
        except ClientError as error:
            if error.response["Error"]["Code"] == "ConditionalCheckFailedException":
                print("Registro duplicado; no se almacena nuevamente")
                return {"statusCode": 200, "message": "Duplicado"}
            raise

        print(f"Registro guardado: {thing} - {tipo}")

        return {
            "statusCode": 200,
            "message": "Registro almacenado",
            "timestampId": item["timestampId"]
        }

    except (ValueError, TypeError, OverflowError) as error:
        print(f"Mensaje descartado: {error}")
        return {
            "statusCode": 400,
            "message": str(error)
        }
