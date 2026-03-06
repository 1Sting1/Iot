#define DIR_LEFT 4
#define SPEED_LEFT 5

#define DIR_RIGHT 7
#define SPEED_RIGHT 6

#define FORWARD_LEFT HIGH
#define BACKWARD_LEFT LOW
#define FORWARD_RIGHT LOW
#define BACKWARD_RIGHT HIGH

void move(int left_dir, int left_speed, int right_dir, int right_speed) {
    digitalWrite(DIR_LEFT, left_dir);
    digitalWrite(DIR_RIGHT, right_dir);
    analogWrite(SPEED_LEFT, left_speed);
    analogWrite(SPEED_RIGHT, right_speed);
}

void forward(int speed) {
    move(FORWARD_LEFT, speed, FORWARD_RIGHT, speed);
}

void backward(int speed) {
    move(BACKWARD_LEFT, speed, BACKWARD_RIGHT, speed);
}

void turn_left(int steepness) {
    move(FORWARD_LEFT, 0, FORWARD_RIGHT, steepness);
}

void turn_right(int steepness) {
    move(FORWARD_LEFT, steepness, FORWARD_RIGHT, 0);
}

void rotate_left(int speed) {
    move(BACKWARD_LEFT, speed, FORWARD_RIGHT, speed);
}

void rotate_right(int speed) {
    move(FORWARD_LEFT, speed, BACKWARD_RIGHT, speed);
}

void stop_motors() {
    move(FORWARD_LEFT, 0, FORWARD_RIGHT, 0);
}

void setup() {
    pinMode(DIR_LEFT, OUTPUT);
    pinMode(DIR_RIGHT, OUTPUT);
    pinMode(SPEED_LEFT, OUTPUT);
    pinMode(SPEED_RIGHT, OUTPUT);
}

void loop() {

    forward(255);
    delay(2000);

    turn_left(255);
    delay(1500);

    forward(255);
    delay(2000);

    rotate_right(255);
    delay(2000);

    backward(255);
    delay(2000);

    stop_motors();
    delay(3000);
}