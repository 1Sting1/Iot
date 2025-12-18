#include <avr/io.h>
#include <avr/interrupt.h>

int latchPin = 5;
int clockPin = 3;
int dataPin = 7;

bool digits[10][8] = {
  {1,1,0,1,1,1,0,1},
  {0,1,0,1,0,0,0,0},
  {1,1,0,0,1,1,1,0},
  {1,1,0,1,1,0,1,0},
  {0,1,0,1,0,0,1,1},
  {1,0,0,1,1,0,1,1},
  {1,0,1,1,1,1,1,1},
  {1,1,0,1,0,0,0,0},
  {1,1,0,1,1,1,1,1},
  {1,1,1,1,1,0,1,1}
};

volatile int internal_counter = 0;
volatile int override_value = -1;
volatile bool system_running = false;

volatile uint16_t shift_buffer = 0;
volatile int bit_index = 15;
volatile int ms_counter = 0;

enum State { STATE_WAIT, STATE_SHIFT, STATE_LATCH };
volatile State current_state = STATE_WAIT;

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  digitalWrite(clockPin, LOW);
  digitalWrite(latchPin, HIGH);

  Serial.begin(9600);
  Serial.println(F("Send start value"));

  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 249;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void loop() {
  static char input_buf[3];
  static byte pos = 0;

  if (Serial.available()) {
    char c = Serial.read();

    if (c >= '0' && c <= '9') {
      if (pos < 2) input_buf[pos++] = c;
    }

    if (pos == 2 || c == '\n') {
      input_buf[pos] = 0;
      int val = atoi(input_buf);

      if (val >= 0 && val <= 99) {
        cli();

        if (!system_running) {
          internal_counter = val;
          system_running = true;
          ms_counter = 999;
          Serial.print(F("STARTED at: ")); Serial.println(val);
        } else {
          override_value = val;
          Serial.print(F("OVERRIDE next frame with: ")); Serial.println(val);
        }

        sei();
      }
      pos = 0;
    }
  }
}

uint16_t encode_number(int number) {
  int tens = number / 10;
  int ones = number % 10;

  uint16_t bits = 0;

  for(int i=7; i>=0; i--) {
     if(digits[tens][i]) bits |= (1 << (8 + i));
  }

  for(int i=7; i>=0; i--) {
     if(digits[ones][i]) bits |= (1 << i);
  }

  return bits;
}

ISR(TIMER1_COMPA_vect) {
  ms_counter++;

  if (ms_counter >= 1000) {
    ms_counter = 0;

    if (system_running) {
      int value_to_show;

      if (override_value != -1) {
        value_to_show = override_value;
        override_value = -1;
        internal_counter++;
      }
      else {
        value_to_show = internal_counter;
        internal_counter++;
      }

      if (internal_counter > 99) internal_counter = 0;

      shift_buffer = encode_number(value_to_show);
      current_state = STATE_SHIFT;
      bit_index = 15;
    }
  }
  switch (current_state) {
    case STATE_SHIFT:
      if ((shift_buffer >> bit_index) & 1) {
        PORTD |= (1 << 7);
      } else {
        PORTD &= ~(1 << 7);
      }
      PORTD |= (1 << 3);
      PORTD &= ~(1 << 3);

      bit_index--;
      if (bit_index < 0) {
        current_state = STATE_LATCH;
      }
      break;

    case STATE_LATCH:
      PORTD |= (1 << 5);
      PORTD &= ~(1 << 5);

      current_state = STATE_WAIT;
      break;

    case STATE_WAIT:
      break;
  }
}