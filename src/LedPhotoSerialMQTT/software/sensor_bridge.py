import serial
import time
import random
from paho.mqtt.client import Client

SERIAL_PORT = 'COM8'
BAUD_RATE = 9600
BROKER = "broker.emqx.io"
TOPIC_LUMINOSITY = "laboratory/greenhouse/luminosity"
CLIENT_ID = f'SENSOR_PC_{random.randint(1000, 9999)}'

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    print(f"Connected to Sensor MCU on {SERIAL_PORT}")
except Exception as e:
    print(f"Error connecting to Serial: {e}")
    exit()


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print(f"Failed to connect, return code {rc}")


client = Client(client_id=CLIENT_ID)
client.on_connect = on_connect

client.connect(BROKER)
client.loop_start()

try:
    while True:
        ser.write(b'p')

        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if "SENSOR_VALUE:" in line:
                try:
                    val = line.split(":")[1]
                    client.publish(TOPIC_LUMINOSITY, val, qos=1)
                    print(f"[Sensor PC] Read: {val} -> Published")
                except IndexError:
                    pass

        time.sleep(2)

except KeyboardInterrupt:
    print("Stopping...")
    ser.close()
    client.disconnect()
    client.loop_stop()