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


void setupTimer() {
	//Timer 0 konfigurieren
	TCCR1A = (1<<WGM01); // CTC Modus
	TCCR1B |= (1<<CS01); // Prescaler 8
	// ((8000000/8)/1000) = 125
	// OCR1A = 125-1;

	// Compare Interrupt erlauben
	// TIMSK |= (1<<OCIE0A);
}

void setupInts() {
	// Hardware Interrupts
	GIMSK |= 0b11000000; // Int1 und Int0 aktivieren
	MCUCR |= (1 << ISC11) + (0 << ISC10) + (1 << ISC01) + (0 << ISC00); //fallende flanke
	// Timer interrupts
	// Interrupts global aktivieren
	sei();

}

ISR(INT0_vect) {

}

void set_leds(uint8_t bot, uint8_t top) {
    uint8_t a, b, d;
    a = top;
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
    PORTD = d;
}



int main (void) {
	DDRA  = 0b00000011; // 5pins not accessible,Reset,LED- 0-1
	PORTA = 0b00000000;

	DDRB  = 0b00011111; // SCK,MISO,MOSI,LED+ 0-4 (1=Out, 0=In)
	PORTB = 0b00000000;

	DDRD  = 0b01000011; // not acessible, nc, Button, nc, 2*sensDir, 2*receive
	PORTD = 0b00000000;

    PORTA = 0b00000011;
    PORTB = 0b00011111;
    PORTD = 0b01000011;

	setupTimer();
	setupInts(); // setup the interrupt routines for the wave detection

    _delay_ms(70);

	set_leds(0, 0);
	/*_delay_ms(50);
	set_leds(1 << 0, 0);
	_delay_ms(50);
	set_leds(1 << 1, 0);
	_delay_ms(50);
	set_leds(1 << 2, 0);
	_delay_ms(50);
	set_leds(1 << 3, 0);
	_delay_ms(50);
	set_leds(1 << 4, 0);
	_delay_ms(50);
	set_leds(1 << 5, 0);
	_delay_ms(50);
	set_leds(1 << 6, 0);
	_delay_ms(50);
	set_leds(1 << 7, 0);
	_delay_ms(50);
	set_leds(0, 1 << 0);
	_delay_ms(50);
	set_leds(0, 1 << 1);
	_delay_ms(50);*/



    uint8_t chr = 0;
    while(1) {
        uint8_t c = eeprom_read_byte(&eemem[chr])-0x30;

        for(uint8_t x = 0; x < 5; ++x) {
            uint8_t bot = pgm_read_byte(&font_bottom[c*FONT_CHAR_WIDTH+x]);
            uint8_t top = pgm_read_byte(&font_top[(c*FONT_CHAR_WIDTH+x) >> 2]);
            top >>= (6-(x << 1));
            set_leds(bot, top);
            _delay_ms(1);
        }

        ++chr;
        if(chr == 11)
            chr = 0;
    }
    /*
	while(1) {
		_delay_ms(30);
		uint8_t off = (*chr) - 0x30;
		uint8_t bot = font_bottom[off*FONT_CHAR_WIDTH+x];
		uint8_t top = font_top[(off*FONT_CHAR_WIDTH+x) >> 2];
		top = top >> (6-2*x);

        set_leds(bot, 0);

        x++;
        if (x == FONT_CHAR_WIDTH) {
            chr++;
            x = 0;
        }

        if (chr - array[0] >= 10) {
            chr = array[0];
            x = 0;
        }
	}

/*
  while (1==1) {
    //waitms(100);
		if BUTTON {
			PORTB = 0b00000000;
			PORTA |= 0b00000011;
		}else if ((state==0) && !(PIND & 1<<2)) {
			state = 1;
			count++;
			PORTB = ~(count >> 1) & 0b00011111;
			if (count & 1) {
				PORTA |= 1 << 1;
				PORTA &= ~(1 << 0);
			} else {
				PORTA |= 1 << 0;
				PORTA &= ~(1 << 1);
			}
		} else if ((state==1) && !(PIND & 1<<3)) {
			state = 0;
			count++;
			PORTB = ~(count >> 1) & 0b00011111;
			if (count & 1) {
				PORTA |= 1 << 1;
				PORTA &= ~(1 << 0);
			} else {
				PORTA |= 1 << 0;
				PORTA &= ~(1 << 1);
			}
		} else { 
			PORTB = ~(count >> 1) & 0b00011111;
			if (count & 1) {
				PORTA |= 1 << 1;
				PORTA &= ~(1 << 0);
			} else {
				PORTA |= 1 << 0;
				PORTA &= ~(1 << 1);
			}
		}
  }*/
}

void waitms(int ms) {
  int i;
  for (i=0;i<ms;i++) _delay_ms(1);
}

