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

ISR(TIMER1_COMPA_vect) {
    x++;
    if (x == FONT_CHAR_WIDTH) {
        x = 0;
        chr++;
        if (chr == 11) {
				    //Disable Timer
            TCCR1B = 0;
			      //Disable timer Interrupts
            TIMSK &= ~(1 << OCIE1A);
						// Disable LEDs
            x = -1;
        }
    }
		// increment counter
    uint32_t timer = TCNT1L;          // read counter
    timer |= OCR1AH << 8;
		timer += columnduration;
    OCR1AH = timer >> 8;
    OCR1AL = timer & 0xff;
}

ISR(TIMER1_OVF_vect) {
	// set controller to sleep
	columnduration = 0xff;
}

ISR(PCINT2_vect) {
    if (PCMSK2 == 0b00001000 && (PIND & 0b00001000) == 0) {
        // forward
        // reset cursors
        chr = 0;
        x = -1;

        // read counter
        TCCR1B = 0;                         // disable timer
        uint32_t timeout = TCNT1L;          // read counter
        timeout |= TCNT1H << 8;
				// a full cycle gets measured and in the beginning and the end a char is omitted
        //timeout /= (26 * FONT_CHAR_WIDTH);    // divide counter to get update intervall
        timeout >>= 7;    // divide counter to get update intervall
				// TODO: approximate by bit shift, low pass?
        columnduration = timeout & 0xff;
				// setup timer
        TCNT1H = 0;                         // clear timer
        TCNT1L = 0;
				TCCR1A = 0;                         // normal counter mode
				// set delay time
				timeout <<= 3;											// 8 columns delay
        OCR1AH = timeout >> 8;
        OCR1AL = timeout & 0xff;
				// start timer
        TIMSK |= (1 << OCIE1A);               // enable timer compare interrupt
        TIMSK |= (1 << TOIE1);               // enable timer overflow interrupt
        TCCR1B = (1 << CS12) | (1 << CS10);     // re-enable using prescaler 1024 (gives about 839 ms time to overflow and 128 us resolution)

        // next interrupt on other side
        PCMSK2 = 0b00010000;
    } else if (PCMSK2 == 0b00010000 && (PIND & 0b00010000) == 0) {
        // backward
        // setup timer as counter
        TIMSK &= ~(1 << OCIE1A);  // disable timer compare interrupt
        TCCR1B = (1 << CS12) | (1 << CS10);     // re-enable using prescaler 1024 (gives about 839 ms time to overflow and 128 us resolution)

        // disable LEDs
				x = -1;

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
			}
    }
}
