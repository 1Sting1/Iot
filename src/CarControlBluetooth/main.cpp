#include <SoftwareSerial.h>

const byte pinRPwm = 5;
const byte pinRDir = 4;
const byte pinLPwm = 6;
const byte pinLDir = 7;

const int cruiseSpeed = 180;
const int turnSpeed = 200;

SoftwareSerial bluetooth(10, 11);

void setDrive(bool leftDir, byte leftSpd, bool rightDir, byte rightSpd) {
    digitalWrite(pinLDir, leftDir);
    analogWrite(pinLPwm, leftSpd);
    digitalWrite(pinRDir, rightDir);
    analogWrite(pinRPwm, rightSpd);
}

void goForward()  { setDrive(HIGH, cruiseSpeed, HIGH, cruiseSpeed); }
void goBackward() { setDrive(LOW, cruiseSpeed, LOW, cruiseSpeed); }
void brake()      { setDrive(LOW, 0, LOW, 0); }

void spinLeft()   { setDrive(LOW, turnSpeed, HIGH, turnSpeed); }
void spinRight()  { setDrive(HIGH, turnSpeed, LOW, turnSpeed); }

void curveLeft(int factor)  { setDrive(HIGH, factor, HIGH, 255); }
void curveRight(int factor) { setDrive(HIGH, 255, HIGH, factor); }

void setup() {
    pinMode(pinRPwm, OUTPUT);
    pinMode(pinRDir, OUTPUT);
    pinMode(pinLPwm, OUTPUT);
    pinMode(pinLDir, OUTPUT);

    Serial.begin(57600);
    bluetooth.begin(9600);

    Serial.println(F("System Started"));
    bluetooth.println(F("RC Robot: Online"));

    brake();
}

void handleInput(char sign) {
    if (sign >= 'a' && sign <= 'z') sign -= 32;

    switch (sign) {
        case 'F': goForward();  break;
        case 'B': goBackward(); break;
        case 'L': spinLeft();   break;
        case 'R': spinRight();  break;
        case 'S': brake();      break;
        default:  break;
    }
}

void loop() {
    if (bluetooth.available() > 0) {
        char incoming = bluetooth.read();

        if (incoming > 32) {
            handleInput(incoming);
        }
    }
}
