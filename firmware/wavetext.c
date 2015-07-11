/*
 wavetext - a very affordable POV Display

 - 10 LEDs
 - 1 ATtiny2313
 - some resistors
 - Clock: 8MHz (internal)
*/

#define F_CPU 8000000UL  // 8 MHz

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "font.h"


const uint8_t eemem[] EEMEM = "Freakquenz{Chaos{DA{{{Socke{{{{{{Wirew0rm{{";

void setupInterrupts() {
    GIMSK |= 0b00010000;    // allow pcint2
    PCMSK2 = 0b00001000;    // only on this pin
    sei();                  // enable interrupts
}

void waitms(int ms) {
    while (ms--) {
        _delay_ms(1);
    }
}

void waitus(int us) {
    while (us--) {
        _delay_us(1);
    }
}

void set_leds(uint8_t bot, uint8_t top) {
    uint8_t a, b, d;
    a = top;
    b = bot & 0b00011111;
    d = (bot & 0b00100000) << 1;
    d |= (bot & 0b11000000) >> 6;

    PORTA = a;
    PORTB = b;
    PORTD = 0b00111000 | d;
}

volatile uint8_t chr = 0;
volatile uint8_t x = 0;
volatile uint8_t mode = 0;
ISR(TIMER1_COMPA_vect) {
    x++;
    if (x == FONT_CHAR_WIDTH) {
        x = 0;
        chr++;

        if (chr == 11) {
            TCCR1B = 0;
            TIMSK &= ~(1 << OCIE1A);
            mode = 0;
        }
    }
}


ISR(PCINT2_vect) {
    if (PCMSK2 == 0b00001000 && (PIND & 0b00001000) == 0) {
        // forward
        // reset cursors
        chr = 0;
        x = 0;
        mode = 0xff;

        // read counter
        TCCR1B = 0;                         // disable timer
        TCCR1A = (1 << WGM11);                // ctc modus with OCR1A
        uint32_t timeout = TCNT1L;          // read counter
        timeout |= TCNT1H << 8;
        timeout <<= 7;                      // convert to microsecs
        timeout /= (11 * FONT_CHAR_WIDTH);    // divide counter to get update intervall
        //timeout >>= 4;                      // double the gun, double the fun!
        //timeout = 2;
        OCR1AH = timeout >> 8;
        OCR1AL = timeout & 0xff;
        //OCR1AH = 0;
        //OCR1AL = 250;
        TCNT1H = 0;                         // clear timer
        TCNT1L = 0;
        waitus(FONT_CHAR_WIDTH * timeout);
        TIMSK |= (1 << OCIE1A);               // enable timer compare interrupt
        TCCR1B = (1 << CS11);                 // re-enable using prescaler 8

        // next interrupt on other side
        PCMSK2 = 0b00010000;
    } else if (PCMSK2 == 0b00010000 && (PIND & 0b00010000) == 0) {
        // backward
        // setup timer as counter
        TCCR1B = 0;             // disable timer
        TIMSK &= ~(1 << OCIE1A);  // disable timer compare interrupt
        TCCR1A = 0;             // set to counter mode
        TCNT1H = 0;             // reset value to 0
        TCNT1L = 0;             // ...
        TCCR1B = (1 << CS12) | (1 <<
                                CS10);     // re-enable using prescaler 1024 (gives about 839 ms time to overflow and  128 us resolution)

        // disable LEDs
        mode = 0;

        // next interrupt on other side
        PCMSK2 = 0b00001000;
    }
}


int main(void) {
    DDRA = 0b00000011; // 5pins not accessible,Reset,LED- 0-1
    DDRB = 0b00011111; // SCK,MISO,MOSI, 5xLED (1=Out, 0=In)
    DDRD = 0b01000011; // not acessible, led, sw1, 2*sensDir, 3x led
    set_leds(0, 0);


    set_leds(0b11111111, 0b00000011);
    _delay_ms(70);
    set_leds(0, 0);

    // initialize
    chr = 0;
    x = 0;

    //setupTimer();
    setupInterrupts(); // setup the interrupt routines for the wave detection

    while (1) {
        uint8_t c = eeprom_read_byte(&eemem[chr]) - 0x30;
        uint8_t bot = pgm_read_byte(&font_bottom[c * FONT_CHAR_WIDTH + x]);
        uint8_t top = pgm_read_byte(&font_top[(c * FONT_CHAR_WIDTH + x) >> 2]);
        top >>= (6 - (x << 1));
        set_leds(bot & mode, top & mode);
    }
}

