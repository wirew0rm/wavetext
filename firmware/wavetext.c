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

void setLEDs(uint8_t bot, uint8_t top) {
    uint8_t a, b, d;
    a = top;
    b = bot & 0b00011111;
    d = (bot & 0b00100000) << 1;
    d |= (bot & 0b11000000) >> 6;

    PORTA = a;
    PORTB = b;
    PORTD = 0b00111000 | d;
}

//index of currently active character
volatile uint8_t chr = 0;
//xpos in currently active character (-1 = inactive)
volatile int8_t x = -1;
//duraction for one column in counter ticks
volatile uint8_t columnduration = 0xff;

ISR(TIMER0_COMPA_vect) {
    x++;
    if (x == FONT_CHAR_WIDTH) {
        x = 0;
        chr++;
        if (chr == 11) {
			      //Disable timer Interrupts
            TIMSK &= ~(1 << OCIE0A);
						// Disable LEDs
            x = -1;
        }
    }
}

ISR(TIMER1_COMPA_vect) {
	// set controller to sleep
	TCCR1B = 0;
}

ISR(PCINT2_vect) {
    if (PCMSK2 == 0b00001000 && (PIND & 0b00001000) == 0) {
        // forward
        // reset cursors
        chr = 0;
        x = -14;
        // read counter
        TCCR1B = 0;                         // disable timer
        uint32_t timeout = TCNT1L;          // read counter
        timeout |= TCNT1H << 8;
				// a full cycle gets measured and in the beginning and the end a char is omitted
        //timeout /= (26 * FONT_CHAR_WIDTH);    // divide counter to get update intervall
        timeout >>= 8;    // divide counter to get update intervall
        columnduration = timeout & 0xff;
				// setup timer
        TCNT0  = 0;
        TCNT1H = 0;                         // clear timer
        TCNT1L = 0;
				// set delay time
        OCR0A = timeout;
				// start timer
        TIMSK |= (1 << OCIE1A);               // enable timer1 compare A interrupt
        TIMSK |= (1 << OCIE0A);               // enable timer0 compare A interrupt
        TCCR1B = (1 << CS12) | (1 << CS10);     // re-enable using prescaler 1024 (gives about 839 ms time to overflow and 128 us resolution)
        TCCR0B = (1 << CS12) | (1 << CS10);     // re-enable using prescaler 1024 (gives about 839 ms time to overflow and 128 us resolution)
        // next interrupt on other side
        PCMSK2 = 0b00010000;
    } else if (PCMSK2 == 0b00010000 && (PIND & 0b00010000) == 0) {
        // backward
        TIMSK &= ~(1 << OCIE1A);  // disable timer compare interrupt
				x = -1;                   // disable LEDs
        // next interrupt on other side
        PCMSK2 = 0b00001000;
    }
}

int main(void) {
		// Setup Direction of IO-Pins
    DDRA = 0b00000011; // 5pins not accessible,Reset,LED- 0-1
    DDRB = 0b00011111; // SCK,MISO,MOSI, 5xLED (1=Out, 0=In)
    DDRD = 0b01000011; // not acessible, led, sw1, 2*sensDir, 3x led

		// Blink all LEDs to indicate proper device operation
    setLEDs(0b11111111, 0b00000011);
    _delay_ms(70);
    setLEDs(0, 0);

		// setup timers
    OCR1AH = 0b01111111;                // set sleep delay
    OCR1AL = 0b11111111;
		TCCR1A = 0;                         // normal counter mode
		TCCR0A |= 2;                        // timer0 ctc mode

    // initialize
    chr = 0;
    x = 0;

		// setup the interrupt routines for the wave detection
    setupInterrupts();

		//main loop
    while (1) {
			if (x >= 0) {
        uint8_t c = eeprom_read_byte(&eemem[chr]) - 0x30;
        uint8_t bot = pgm_read_byte(&font_bottom[c * FONT_CHAR_WIDTH + x]);
        uint8_t top = pgm_read_byte(&font_top[(c * FONT_CHAR_WIDTH + x) >> 2]);
        top >>= (6 - (x << 1));
        setLEDs(bot, top);
			} else {
				setLEDs(0,0);
			}
    }
}
