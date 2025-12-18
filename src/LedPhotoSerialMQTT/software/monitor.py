import time
import random
from paho.mqtt.client import Client, MQTTMessage
from paho.mqtt.enums import CallbackAPIVersion

BROKER = "broker.emqx.io"
TOPIC_ROOT = "laboratory/greenhouse/#"
CLIENT_ID = f'MONITOR_PC_{random.randint(1000, 9999)}'

def on_connect(client, userdata, flags, rc, properties):
    print(f"Monitor connected with result code {rc}")
    client.subscribe(TOPIC_ROOT)
    print(f"Monitoring topics: {TOPIC_ROOT}")


def on_message(client, userdata, message: MQTTMessage):
    timestamp = time.strftime("%H:%M:%S", time.localtime())
    topic = message.topic
    payload = message.payload.decode("utf-8")
    print(f"[{timestamp}] TOPIC: {topic:<35} | MSG: {payload}")


client = Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=CLIENT_ID)
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER)

try:
    print("--- MONITOR SYSTEM STARTED ---")
    client.loop_forever()
except KeyboardInterrupt:
    print("Monitor Stopped")
    client.disconnect()