#define NROWS 3
#define NCOLS 3
#define BUTTON_COUNT (NROWS * NCOLS)

volatile bool btn_pressed[BUTTON_COUNT] = {false};
volatile bool input_changed = false;
volatile uint8_t current_scan_row = 0;
unsigned long btn_start_time[BUTTON_COUNT] = {0};
bool prev_btn_state[BUTTON_COUNT] = {0};

void setup() {
  Serial.begin(9600);

  DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4);
  DDRD &= ~((1 << DDD5) | (1 << DDD6) | (1 << DDD7));
  PORTD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4);
  PORTD |= (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7);

  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 999;

  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);

  sei();

  Serial.println("System Initialized. Scanning started.");
}

void loop() {
  bool current_snapshot[BUTTON_COUNT];
  bool state_has_difference = false;
  for(int i=0; i<BUTTON_COUNT; i++) {
    current_snapshot[i] = btn_pressed[i];
    if (current_snapshot[i] != prev_btn_state[i]) {
      state_has_difference = true;
    }
  }

  if (state_has_difference) {
    unsigned long current_time = millis();

    for(int i=0; i<BUTTON_COUNT; i++) {
      if (!prev_btn_state[i] && current_snapshot[i]) {
        btn_start_time[i] = current_time;
        Serial.print("Button ");
        Serial.print(i + 1);
        Serial.print(" pressed at: ");
        Serial.print(current_time);
        Serial.println(" ms");
      }
      if (prev_btn_state[i] && !current_snapshot[i]) {
        unsigned long duration = current_time - btn_start_time[i];
        Serial.print("Button ");
        Serial.print(i + 1);
        Serial.print(" released. Duration: ");
        Serial.print(duration);
        Serial.print(" ms. Started at: ");
        Serial.print(btn_start_time[i]);
        Serial.println(" ms");
      }
    }

    Serial.print("Pressed buttons: ");
    bool any = false;
    for(int i=0; i<BUTTON_COUNT; i++) {
      if (current_snapshot[i]) {
        if (any) Serial.print(", ");
        Serial.print(i + 1);
        any = true;
      }
    }
    if (!any) Serial.print("None");
    Serial.println();

    for(int i=0; i<BUTTON_COUNT; i++) {
      prev_btn_state[i] = current_snapshot[i];
    }
  }
  delay(10);
}

ISR(TIMER1_COMPA_vect) {
  PORTD |= (1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4);

  int pin_to_low = current_scan_row + 2;
  PORTD &= ~(1 << pin_to_low);

  uint8_t pin_state = PIND;

  bool p5 = !((pin_state >> 5) & 1);
  btn_pressed[current_scan_row * 3 + 0] = p5;

  bool p6 = !((pin_state >> 6) & 1);
  btn_pressed[current_scan_row * 3 + 1] = p6;

  bool p7 = !((pin_state >> 7) & 1);
  btn_pressed[current_scan_row * 3 + 2] = p7;

  current_scan_row++;
  if (current_scan_row >= NROWS) {
    current_scan_row = 0;
  }
}