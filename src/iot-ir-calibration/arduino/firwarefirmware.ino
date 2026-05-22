const int TRIG_PIN = 9;
const int ECHO_PIN = 10;
const int IR_PIN = A0;
const int LED_PIN = 13;

bool isCalibrating = false;
int calibMin = 0;
int calibMax = 0;
bool visited[200];

float getUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;
}

bool checkDataSufficiency() {
  int totalBins = calibMax - calibMin + 1;
  int filledCount = 0;
  int currentGap = 0;
  int maxGap = 0;

  for (int i = calibMin; i <= calibMax; i++) {
    if (visited[i]) {
      filledCount++;
      if (currentGap > maxGap) {
        maxGap = currentGap;
      }
      currentGap = 0;
    } else {
      currentGap++;
    }
  }
  if (currentGap > maxGap) {
    maxGap = currentGap;
  }

  float coverage = (float)filledCount / totalBins;

  return (coverage >= 0.90) && (maxGap <= 2);
}

float getDistanceIR(int analogVal, float* coeffs, int degree) {
  float distance = 0.0;
  float currentPower = 1.0;

  for (int i = degree; i >= 0; i--) {
    distance += coeffs[i] * currentPower;
    currentPower *= analogVal;
  }
  return distance;
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.startsWith("CAL")) {
      int firstSpace = cmd.indexOf(' ');
      int secondSpace = cmd.indexOf(' ', firstSpace + 1);

      if (firstSpace > 0 && secondSpace > 0) {
        calibMin = cmd.substring(firstSpace + 1, secondSpace).toInt();
        calibMax = cmd.substring(secondSpace + 1).toInt();

        memset(visited, 0, sizeof(visited));
        isCalibrating = true;
        digitalWrite(LED_PIN, LOW);
        Serial.println("START_CALIBRATION");
      }
    }
  }

  if (isCalibrating) {
    float usDist = getUltrasonicDistance();
    int irVal = analogRead(IR_PIN);

    if (usDist >= calibMin && usDist <= calibMax) {
      int distInt = (int)round(usDist);

      if (!visited[distInt]) {
        visited[distInt] = true;
      }

      Serial.print("DATA ");
      Serial.print(irVal);
      Serial.print(" ");
      Serial.println(usDist);

      if (checkDataSufficiency()) {
        isCalibrating = false;
        Serial.println("DONE");

        for (int i = 0; i < 5; i++) {
          digitalWrite(LED_PIN, HIGH);
          delay(200);
          digitalWrite(LED_PIN, LOW);
          delay(200);
        }
        digitalWrite(LED_PIN, HIGH);
      }
    }
    delay(50);
  }
}