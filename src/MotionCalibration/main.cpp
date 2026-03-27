#include <SoftwareSerial.h>

const int DIR1 = 4;
const int SPEED1 = 5;
const int SPEED2 = 6;
const int DIR2 = 7;

SoftwareSerial btSerial(12, 13);

struct MovementConfig {
  bool d1;
  bool d2;
  int p1;
  int p2;
};

MovementConfig drive[4];
int activeMode = 0;
int currentCombo = 0;

void setup() {
  pinMode(DIR1, OUTPUT);
  pinMode(DIR2, OUTPUT);
  pinMode(SPEED1, OUTPUT);
  pinMode(SPEED2, OUTPUT);

  Serial.begin(9600);
  btSerial.begin(9600);

  for(int i=0; i<4; i++) {
    drive[i] = {false, false, 200, 200};
  }

  Serial.println("System Ready. Waiting for Bluetooth commands...");
}

void runMotors(int mode) {
  digitalWrite(DIR1, drive[mode].d1);
  digitalWrite(DIR2, drive[mode].d2);
  analogWrite(SPEED1, drive[mode].p1);
  analogWrite(SPEED2, drive[mode].p2);
}

void stopMotors() {
  analogWrite(SPEED1, 0);
  analogWrite(SPEED2, 0);
}

void loop() {
  if (btSerial.available()) {
    char key = btSerial.read();

    switch (key) {
      case 'F': activeMode = 0; runMotors(activeMode); break; //вперед
      case 'B': activeMode = 1; runMotors(activeMode); break; //назад
      case 'L': activeMode = 2; runMotors(activeMode); break; //влево
      case 'R': activeMode = 3; runMotors(activeMode); break; //вправо
      case '0': stopMotors(); break; //стоп


      case 'T': // треугольник
        currentCombo = (currentCombo + 1) % 4;
        drive[activeMode].d1 = bitRead(currentCombo, 0);
        drive[activeMode].d2 = bitRead(currentCombo, 1);
        runMotors(activeMode);
        Serial.print("Mode "); Serial.print(activeMode);
        Serial.print(" -> New Combo: "); Serial.println(currentCombo);
        break;

      case 'X': // крестик
        stopMotors();
        Serial.println("Direction Confirmed.");
        break;

      case 'S': // квадрат
        drive[activeMode].p1 = min(drive[activeMode].p1 + 5, 255);
        runMotors(activeMode);
        break;

      case 'C': // кружок
        drive[activeMode].p2 = min(drive[activeMode].p2 + 5, 255);
        runMotors(activeMode);
        break;
    }
  }
}