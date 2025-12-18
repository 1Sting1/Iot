import serial
import time
import random
from paho.mqtt.client import Client, MQTTMessage
from paho.mqtt.enums import CallbackAPIVersion

SERIAL_PORT = 'COM4'
BAUD_RATE = 9600
BROKER = "broker.emqx.io"
TOPIC_LUMINOSITY = "laboratory/greenhouse/luminosity"
TOPIC_STATUS = "laboratory/greenhouse/light_status"
CLIENT_ID = f'ACTUATOR_PC_{random.randint(1000, 9999)}'

LIGHT_THRESHOLD = 500

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    print(f"Connected to Actuator MCU on {SERIAL_PORT}")
except Exception as e:
    print(f"Error connecting to Serial: {e}")
    exit()


def on_connect(client, userdata, flags, rc, properties):
    if rc == 0:
        print("Connected to MQTT Broker")
        client.subscribe(TOPIC_LUMINOSITY, qos=1)
        print(f"Subscribed to {TOPIC_LUMINOSITY}")
    else:
        print(f"Failed to connect, return code {rc}")


def on_message(client, userdata, message: MQTTMessage):
    try:
        payload = message.payload.decode("utf-8")
        lux_value = int(payload)
        print(f"[Actuator PC] Received Luminosity: {lux_value}")

        command = ''
        if lux_value < LIGHT_THRESHOLD:
            command = 'u'
        else:
            command = 'd'

        ser.write(command.encode())

        time.sleep(0.1)
        if ser.in_waiting > 0:
            response = ser.readline().decode('utf-8').strip()
            print(f"[Actuator PC] MCU Response: {response}")

            client.publish(TOPIC_STATUS, response, qos=1)

    except ValueError:
        print("Received non-integer data")


client = Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=CLIENT_ID)
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER)

try:
    client.loop_forever()
except KeyboardInterrupt:
    print("Stopping")
    ser.close()
    client.disconnect()