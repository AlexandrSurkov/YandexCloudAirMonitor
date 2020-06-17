import datetime
import logging
import requests
import os
import json
import base64

METRICS_PUSH_URL = 'https://monitoring.api.cloud.yandex.net/monitoring/v2/data/write'
METRICS_SERVICE = 'custom'

logger = logging.getLogger()
logger.setLevel(logging.INFO)

verboseLogging = eval(os.environ['VERBOSE_LOG']) ## Convert to bool

def logInfo(text):
   if verboseLogging:
     logger.Info(text)

logInfo('Loading my-function')

def pushMetrics(iamToken, msg):
    folderId = os.environ["METRICS_FOLDER_ID"]
    metrics = makeAllMetrics(msg)
    logInfo(f'Metrics request: {metrics}')
    resp = requests.post(
        METRICS_PUSH_URL,
        json=metrics,
        headers={"Authorization": "Bearer " + iamToken},
        params={"folderId": folderId, "service": METRICS_SERVICE}
    )
    logInfo(f'Metrics response: {resp}')
    logInfo(f'Metrics response.content: {resp.content}')

"""
Imput Json format is:
{
    "DeviceId":"7d972e16-2cc7-49aa-a3fb-153be9b2e04f",
    "TimeStamp":"2020-05-19T18:41:37.145+03:00",
    "Values":[
        {"Type":"Float","Name":"Humidity","Value":"90.22961"},
        {"Type":"Float","Name":"CarbonDioxide","Value":"125.06672"},
        {"Type":"Float","Name":"Pressure","Value":"32.808365"},
        {"Type":"Float","Name":"Temperature","Value":"31.049744"}
        ]
}
"""
def makeAllMetrics(msg):
    metrics = [
        makeMetric(msg["Values"][0]["Name"], msg["Values"][0]["Value"]),
        makeMetric(msg["Values"][1]["Name"], msg["Values"][1]["Value"]),
        makeMetric(msg["Values"][2]["Name"], msg["Values"][2]["Value"]),
        makeMetric(msg["Values"][3]["Name"], msg["Values"][3]["Value"])
    ]
    ts = msg["TimeStamp"]
    return {
        "ts": ts,
        "labels": {
            "device_id": msg["DeviceId"],
        },
        "metrics": metrics
    }

def makeMetric(name, value):
    return {
        "name": name,
        "type": "DGAUGE",
        "value": float(value),
    }

"""
    Entry-point for Serverless Function.
    :param event: IoT message payload.
    :param context: information about current execution context.
    :return: sucessfull response statusCode: 200
"""
def msgHandler(event, context):
    statusCode = 500  ## Error response by default

    logInfo(event)
    logInfo(context)

    msg_payload = json.dumps(event["messages"][0])
    json_msg = json.loads(msg_payload)
    event_payload = base64.b64decode(json_msg["details"]["payload"])

    logInfo(f'Event: {event_payload}')

    payload_json = json.loads(event_payload)

    iam_token = context.token["access_token"]
    pushMetrics(iam_token, payload_json)

    statusCode = 200
    
    return {
        'statusCode': statusCode,
        'headers': {
            'Content-Type': 'text/plain'
        },
        'isBase64Encoded': False
    }

"""
Data for test:

{
    "messages": [
        {
            "event_metadata": {
                "event_id": "160d239876d9714800",
                "event_type": "yandex.cloud.events.iot.IoTMessage",
                "created_at": "2020-05-08T19:16:21.267616072Z",
                "folder_id": "b112345678910"
            },
            "details": {
                "registry_id": "are1234567890",
                "device_id": "are0987654321",
                "mqtt_topic": "$devices/are0987654321/events",
                "payload": "eyJWYWx1ZXMiOiBbeyJWYWx1ZSI6ICI5MC4yMjk2MSIsICJUeXBlIjogIkZsb2F0IiwgIk5hbWUiOiAiSHVtaWRpdHkifSwgeyJWYWx1ZSI6ICIxMjUuMDY2NzIiLCAiVHlwZSI6ICJGbG9hdCIsICJOYW1lIjogIkNhcmJvbkRpb3hpZGUifSwgeyJWYWx1ZSI6ICIzMi44MDgzNjUiLCAiVHlwZSI6ICJGbG9hdCIsICJOYW1lIjogIlByZXNzdXJlIn0sIHsiVmFsdWUiOiAiMzEuMDQ5NzQ0IiwgIlR5cGUiOiAiRmxvYXQiLCAiTmFtZSI6ICJUZW1wZXJhdHVyZSJ9XSwgIkRldmljZUlkIjogIjdkOTcyZTE2LTJjYzctNDlhYS1hM2ZiLTE1M2JlOWIyZTA0ZiIsICJUaW1lU3RhbXAiOiAiMjAyMC0wNS0xOVQxODo0MTozNy4xNDUrMDM6MDAifQ=="
            }
        }
    ]
}
"""
