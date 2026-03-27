#define SPEED_LEFT  5
#define DIR_LEFT    4
#define SPEED_RIGHT 6
#define DIR_RIGHT   7

#define FORWARD  LOW
#define BACK     HIGH
#define DEFAULT_SPEED   180  
#define ROTATE_SPEED    180  

#define TRIG_LEFT   10
#define ECHO_LEFT   11
#define TRIG_FRONT  8
#define ECHO_FRONT  9

#define FRONT_STOP_DIST     8
#define LEFT_TARGET_DIST    8
#define LEFT_MIN_DIST       8
#define LEFT_MAX_DIST       8
#define HYSTERESIS          1

#define SENSOR_INTERVAL     50
#define TURN_TIMEOUT        2000
#define SERIAL_INTERVAL     100  

enum State {
  STATE_FORWARD,
  STATE_STOPPED,
  STATE_TURNING_RIGHT,
  STATE_WALL_FOLLOW
};

const char* stateNames[] = {
  "FORWARD",
  "STOPPED", 
  "TURNING_RIGHT",
  "WALL_FOLLOW"
};

State currentState = STATE_FORWARD;
unsigned long lastSensorRead = 0;
unsigned long turnStartTime = 0;
unsigned long lastSerialPrint = 0;

int distFront = 255;
int distLeft  = 255;

void move(int dirLeft, int speedLeft, int dirRight, int speedRight) {
  digitalWrite(DIR_LEFT, dirLeft);
  analogWrite(SPEED_LEFT, speedLeft);
  digitalWrite(DIR_RIGHT, dirRight);
  analogWrite(SPEED_RIGHT, speedRight);
}

void forward(int speed)      { move(FORWARD, speed, FORWARD, speed); }
void backward(int speed)     { move(BACK, speed, BACK, speed); }
void turn_left(int steepness){ move(FORWARD, steepness, FORWARD, 255); }
void turn_right(int steepness){ move(FORWARD, 255, FORWARD, steepness); }
void rotate_right(int speed) { move(FORWARD, speed, BACK, speed); }
void rotate_left(int speed)  { move(BACK, speed, FORWARD, speed); }
void stopMotors()            { move(LOW, 0, LOW, 0); }


int readUltrasonic(uint8_t trigPin, uint8_t echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 20000);
  if (duration == 0) return 255;
  return duration / 58;  
}

void updateSensors() {
  distLeft  = readUltrasonic(TRIG_LEFT, ECHO_LEFT);
  distFront = readUltrasonic(TRIG_FRONT, ECHO_FRONT);
}


void printDebug() {
  Serial.print("[" + String(stateNames[currentState]) + "]");

  Serial.print(" Front: ");
  if (distFront >= 255) Serial.print(" --- ");
  else Serial.print(distFront);
  Serial.print(" cm | Left: ");
  if (distLeft >= 255) Serial.print(" --- ");
  else Serial.print(distLeft);
  Serial.print(" cm | Action: ");
  
  switch (currentState) {
    case STATE_FORWARD:       Serial.print(">>> FORWARD"); break;
    case STATE_STOPPED:       Serial.print("||| STOP"); break;
    case STATE_TURNING_RIGHT: Serial.print("⟳ TURN RIGHT"); break;
    case STATE_WALL_FOLLOW:
      if (distLeft < LEFT_MIN_DIST) Serial.print("↘ ADJUST RIGHT");
      else if (distLeft > LEFT_MAX_DIST) Serial.print("↗ ADJUST LEFT");
      else Serial.print(">>> WALL FOLLOW");
      break;
  }
  Serial.println();
}

void stateMachine() {
  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
    updateSensors();
    lastSensorRead = millis();
  }

  if (millis() - lastSerialPrint >= SERIAL_INTERVAL) {
    printDebug();
    lastSerialPrint = millis();
  }

  switch (currentState) {
    
    case STATE_FORWARD:
      if (distFront <= FRONT_STOP_DIST) {
        stopMotors();
        currentState = STATE_STOPPED;
      } else {
        forward(DEFAULT_SPEED);
      }
      break;

    case STATE_STOPPED:
      turnStartTime = millis();
      currentState = STATE_TURNING_RIGHT;
      break;

    case STATE_TURNING_RIGHT:
      rotate_right(ROTATE_SPEED);
      
      if (distLeft <= LEFT_TARGET_DIST + HYSTERESIS && 
          distLeft >= LEFT_TARGET_DIST - HYSTERESIS) {
        currentState = STATE_WALL_FOLLOW;
        break;
      }
      
      if (millis() - turnStartTime > TURN_TIMEOUT) {
        stopMotors();
        currentState = STATE_FORWARD;
      }
      break;

    case STATE_WALL_FOLLOW:
      if (distFront <= FRONT_STOP_DIST) {
        stopMotors();
        currentState = STATE_STOPPED;
        break;
      }
      
      if (distLeft < LEFT_MIN_DIST) {
        turn_right(150);
      } 
      else if (distLeft > LEFT_MAX_DIST) {
        turn_left(150);
      } 
      else {
        forward(DEFAULT_SPEED);
      }
      break;
  }
}


void setup() {
  // Моторы
  pinMode(SPEED_LEFT, OUTPUT);
  pinMode(DIR_LEFT, OUTPUT);
  pinMode(SPEED_RIGHT, OUTPUT);
  pinMode(DIR_RIGHT, OUTPUT);
  
  // Датчики
  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
  
  stopMotors();
  
  Serial.begin(9600);
  while (!Serial);
}

void loop() {
  stateMachine();
}