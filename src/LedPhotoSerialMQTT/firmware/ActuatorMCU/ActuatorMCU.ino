
const int ledPin = 13;
bool isBlinking = false;
unsigned long lastBlinkTime = 0;
bool ledState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    while(Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
      Serial.read();
    }

    if (cmd == 'u') {
      isBlinking = false;
      digitalWrite(ledPin, HIGH);
      Serial.println("LED_GOES_ON");
    }
    else if (cmd == 'd') {
      isBlinking = false;
      digitalWrite(ledPin, LOW);
      Serial.println("LED_GOES_OFF");
    }
    else if (cmd == 'b') {
      isBlinking = true;
      Serial.println("LED_WILL_BLINK");
    }
  }

  if (isBlinking) {
    if (millis() - lastBlinkTime >= 500) {
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      lastBlinkTime = millis();
    }
  }
}