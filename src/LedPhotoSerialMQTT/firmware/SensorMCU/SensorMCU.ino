const int sensorPin = A0;
bool isStreaming = false;
unsigned long lastStreamTime = 0;
const int streamInterval = 1000;

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    while(Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
      Serial.read();
    }

    if (cmd == 'p') {
      int val = analogRead(sensorPin);
      Serial.print("SENSOR_VALUE:");
      Serial.println(val);
    }
    else if (cmd == 's') {
      isStreaming = true;
      Serial.println("STREAM_STARTED");
    }
    else if (cmd == 'q') {
      isStreaming = false;
      Serial.println("STREAM_STOPPED");
    }
  }

  if (isStreaming) {
    if (millis() - lastStreamTime >= streamInterval) {
      int val = analogRead(sensorPin);
      Serial.print("SENSOR_VALUE:");
      Serial.println(val);
      lastStreamTime = millis();
    }
  }
}