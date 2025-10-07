#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <string.h>

#define F_CPU 16000000UL
#define TX_PORT PORTD
#define TX_DDR  DDRD
#define TX_PIN  PD3
#define RX_PORT PORTD
#define RX_PIN_REG PIND
#define RX_DDR  DDRD
#define RX_PIN  PD2
#define UART_BUFFER_SIZE 64
#define DEBUG_MODE 1

volatile char tx_buffer[UART_BUFFER_SIZE];
volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;

volatile char rx_buffer[UART_BUFFER_SIZE];
volatile uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0;

volatile uint16_t tx_shift_register;
volatile uint8_t tx_bit_count;
volatile uint8_t rx_byte;
volatile uint8_t rx_bit_count;
volatile uint16_t timer_ticks_per_bit;

volatile uint32_t tx_byte_count = 0;
volatile uint32_t rx_byte_count = 0;
volatile uint32_t rx_interrupt_count = 0;

static inline uint8_t uart_rx_available() {
    uint8_t count;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        count = (rx_head - rx_tail) % UART_BUFFER_SIZE;
    }
    return count;
}

static inline void buffer_put(volatile char *buffer, volatile uint8_t *head, volatile uint8_t *tail, char data) {
    uint8_t next_head = (*head + 1) % UART_BUFFER_SIZE;
    if (next_head != *tail) {
        buffer[*head] = data;
        *head = next_head;
    }
}

static inline char buffer_get(volatile char *buffer, volatile uint8_t *tail, volatile uint8_t *head) {
    if (*head == *tail) {
        return -1;
    }
    char data = buffer[*tail];
    *tail = (*tail + 1) % UART_BUFFER_SIZE;
    return data;
}

ISR(TIMER1_COMPA_vect) {
    OCR1A += timer_ticks_per_bit;

    if (tx_bit_count > 0) {
        if (tx_shift_register & 0x01) {
            TX_PORT |= (1 << TX_PIN);
        }
        else {
            TX_PORT &= ~(1 << TX_PIN);
        }
        tx_shift_register >>= 1;
        tx_bit_count--;
    }
    else {
        if (tx_head != tx_tail) {
            char next_byte = buffer_get(tx_buffer, &tx_tail, &tx_head);
            tx_shift_register = (next_byte << 1) | 0x200;
            tx_bit_count = 10;

            TX_PORT &= ~(1 << TX_PIN);
            tx_shift_register >>= 1;
            tx_bit_count--;

            tx_byte_count++;
        }
        else {
            TIMSK1 &= ~(1 << OCIE1A);
            TX_PORT |= (1 << TX_PIN);
        }
    }
}

ISR(TIMER1_COMPB_vect) {
    OCR1B += timer_ticks_per_bit;

    if (rx_bit_count < 8) {
        rx_byte >>= 1;

        if (RX_PIN_REG & (1 << RX_PIN)) {
            rx_byte |= 0x80;
        }
        rx_bit_count++;
    }
    else {
        TIMSK1 &= ~(1 << OCIE1B);
        buffer_put(rx_buffer, &rx_head, &rx_tail, rx_byte);
        rx_byte_count++;

        EIFR |= (1 << INTF0);
        EIMSK |= (1 << INT0);
    }
}

ISR(INT0_vect) {
    EIMSK &= ~(1 << INT0);
    OCR1B = TCNT1 + timer_ticks_per_bit + (timer_ticks_per_bit / 2);

    rx_byte = 0;
    rx_bit_count = 0;
    rx_interrupt_count++;

    TIFR1 |= (1 << OCF1B);
    TIMSK1 |= (1 << OCIE1B);
}

void uart_set_baudrate(long rate) {
    TX_DDR |= (1 << TX_PIN);
    TX_PORT |= (1 << TX_PIN);
    RX_DDR &= ~(1 << RX_PIN);
    RX_PORT |= (1 << RX_PIN);

    uint32_t prescaler = 8;
    timer_ticks_per_bit = (F_CPU / prescaler) / rate;

    TCCR1A = 0;
    TCCR1B = (1 << CS11);
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
    sei();
}

void uart_send(char b) {
    uint8_t next_head = (tx_head + 1) % UART_BUFFER_SIZE;

    while (next_head == tx_tail) {
    }
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        bool is_tx_idle = (TIMSK1 & (1 << OCIE1A)) == 0;
        buffer_put(tx_buffer, &tx_head, &tx_tail, b);

        if (is_tx_idle) {
            OCR1A = TCNT1 + timer_ticks_per_bit;
            TIMSK1 |= (1 << OCIE1A);
        }
    }
}

void uart_send_string(const char *msg) {
    while (*msg) {
        uart_send(*msg++);
    }
}

void uart_send_number(uint32_t num) {
    char buffer[12];
    ultoa(num, buffer, 10);
    uart_send_string(buffer);
}

uint8_t uart_available() {
    return uart_rx_available();
}

char uart_read() {
    char data;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        data = buffer_get(rx_buffer, &rx_tail, &rx_head);
    }
    return data;
}

bool uart_read_string(char *rx_data) {
    if (uart_available() == 0) return false;

    uint8_t i = 0;
    while (uart_available()) {
        rx_data[i++] = uart_read();
    }
    rx_data[i] = '\0';
    return true;
}

void print_debug_stats() {
    uart_send_string("TX bytes: ");
    uart_send_number(tx_byte_count);
    uart_send_string("\nRX bytes: ");
    uart_send_number(rx_byte_count);
    uart_send_string("\nRX interrupts: ");
    uart_send_number(rx_interrupt_count);
    uart_send_string("\nBuffer available: ");
    uart_send_number(uart_available());
}

void setup() {
    uart_set_baudrate(9600);
    uart_send_string("\n\nUART Initialized!\n");
    uart_send_string("F_CPU: 16000000 Hz\n");
    uart_send_string("Baudrate: 9600\n");
    uart_send_string("TX Pin: D3 (PD3)\n");
    uart_send_string("RX Pin: D2 (PD2/INT0)\n");
    uart_send_string("Buffer size: 64 bytes\n");
    uart_send_string("Ticks per bit: ");
    uart_send_number(timer_ticks_per_bit);
    uart_send_string("\n");
    uart_send_string("Ready for communication!\n");
    uart_send_string("Type to test echo...\n\n");
}

void loop() {
    static uint32_t last_debug_time = 0;
    static uint32_t loop_counter = 0;

    loop_counter++;

    if (DEBUG_MODE && (loop_counter % 10000 == 0)) {
        print_debug_stats();
        last_debug_time = loop_counter;
    }

    if (uart_available()) {
        char received_char = uart_read();

        if (DEBUG_MODE) {
            uart_send_string("[RX: '");
            uart_send(received_char);
            uart_send_string("' (0x");
            char hex[3];
            itoa(received_char, hex, 16);
            uart_send_string(hex);
            uart_send_string(")] ");
        }
        uart_send(received_char);

        if (received_char == '?') {
            uart_send_string("? - Show help\n");
            uart_send_string("s - Show statistics\n");
            uart_send_string("t - Send test pattern\n");
            uart_send_string("r - Reset counters\n\n");
        }
        else if (received_char == 's') {
            print_debug_stats();
        }
        else if (received_char == 't') {
            uart_send_string("\nTest\n");
        }
        else if (received_char == 'r') {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                tx_byte_count = 0;
                rx_byte_count = 0;
                rx_interrupt_count = 0;
            }
            uart_send_string("\nCounters reset!\n");
        }
    }
}