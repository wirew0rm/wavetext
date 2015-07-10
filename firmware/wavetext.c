/*

	 wavetext - a very affordable POV Display

	 10 LEDs
	 1 ATtiny2313
	 some resistors

	 Clock: 8MHz (internal)

*/

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
//#include <avr/iotn2313a.h>

#define F_CPU 8000000UL  // 8 MHz

#include <util/delay.h>


#include "font.h"

#define BUTTON !(PIND & 1<<5)


void waitms(int ms);

/*const char eemem[10][11] EEMEM = 
{
	   "0123456789:", "CHAOS DA", "WALDECK", "FREAKQUENZ", "TEST TEST",
		 "Waldeck", "wirew0rm", "Rock on!", "DARMSTADT", "DONT PANIC!"
};*/
const uint8_t eemem[] EEMEM = "0123456789:";

inline void setupInts() {
	// Hardware Interrupts
    GIMSK |= 0b00010000;    // pcint2 erlauben
    PCMSK2 = 0b00001000;    // nur auf diesen 2 pins
	sei();                  // interrupts anschalten
}

void set_leds(uint8_t bot, uint8_t top) {
    uint8_t a, b, d;
    a  = (top & 0b00000001) << 1;
    a |= (top & 0b00000010) >> 1;
    b  = (bot & 0b00000001) << 4;
    b |= (bot & 0b00000010) << 2;
    b |= (bot & 0b00000100) << 0;
    b |= (bot & 0b00001000) >> 2;
    b |= (bot & 0b00010000) >> 4;
    d  = (bot & 0b00100000) << 1;
    d |= (bot & 0b01000000) >> 6;
    d |= (bot & 0b10000000) >> 6;

    PORTA = a;
    PORTB = b;
    PORTD = 0b00111000 | d;
}

volatile uint8_t chr = 0;
volatile uint8_t x = 0;
ISR(TIMER1_COMPA_vect) {
    uint8_t c = eeprom_read_byte(&eemem[chr])-0x30;
    uint8_t bot = pgm_read_byte(&font_bottom[c*FONT_CHAR_WIDTH+x]);
    uint8_t top = pgm_read_byte(&font_top[(c*FONT_CHAR_WIDTH+x) >> 2]);
    top >>= (6-(x << 1));
    set_leds(bot, top);

    x++;
    if(x == FONT_CHAR_WIDTH) {
        x = 0;
        chr++;

        if(chr == 11) {
            chr = 0;
        }
    }
}


ISR(PCINT2_vect) {
    if (PCMSK2 == 0b00001000 && (PIND & 0b00001000)) {
        // forward
        // read counter
        TCCR1B = 0;                         // disable timer
        TCCR1A = (1<<WGM11);                // ctc modus with OCR1A
        uint32_t counter = TCNT1L;          // read counter
        counter |= TCNT1H << 8;
        counter <<= 5;                      // convert to us
        counter /= (11*FONT_CHAR_WIDTH);    // divide counter to get update timeout
        OCR1AH = counter >> 8;
        OCR1AL = counter & 0xff;
        //TCNT1H = 0;                         // clear timer
        //TCNT1L = 0;
        TIMSK |= (1<<OCIE1A);               // enable timer compare interrupt
        TCCR1B = (1<<CS11);                 // re-enable using prescaler 8

        // enable LEDs
        DDRA = 0b00000011;
        DDRB = 0b00011111;
        DDRD = 0b01000011;

        // next interrupt on other side
        PCMSK2 = 0b00010000;
    } else if (PCMSK2 == 0b00010000 && (PIND & 0b00010000)) {
        // backward
        // setup timer as counter
        TCCR1B = 0;             // disable timer
        TIMSK &= ~(1<<OCIE1A);  // disable timer compare interrupt
        TCCR1A = 0;             // set to counter mode
        TCNT1H = 0;             // reset value to 0
        TCNT1L = 0;             // ...
        TCCR1B = (1<<CS12) | (0<<CS10);     // re-enable using prescaler 1024 (gives about 839 ms time to overflow and  128 us resolution)


        // disable LEDs
        DDRA = 0;
        DDRB = 0;
        DDRD = 0;

        // next interrupt on other side
        PCMSK2 = 0b00001000;
    }
}


int main (void) {
	DDRA  = 0b00000011; // 5pins not accessible,Reset,LED- 0-1
	DDRB  = 0b00011111; // SCK,MISO,MOSI, 5xLED (1=Out, 0=In)
	DDRD  = 0b01000011; // not acessible, led, sw1, 2*sensDir, 3x led 
    set_leds(0, 0);


    set_leds(0b11111111, 0b00000011);
    _delay_ms(70);
	set_leds(0, 0);

    // initialize
    chr = 0;
    x = 0;

	//setupTimer();
	setupInts(); // setup the interrupt routines for the wave detection

    while(1) {};

    /*uint8_t chr = 0;
    while(1) {
        uint8_t c = eeprom_read_byte(&eemem[chr])-0x30;

        for(uint8_t x = 0; x < 5; ++x) {
            uint8_t bot = pgm_read_byte(&font_bottom[c*FONT_CHAR_WIDTH+x]);
            uint8_t top = pgm_read_byte(&font_top[(c*FONT_CHAR_WIDTH+x) >> 2]);
            top >>= (6-(x << 1));
            set_leds(bot, top);
            _delay_us(250);
        }

        ++chr;
        if(chr == 11)
            chr = 0;
    }*/

}
