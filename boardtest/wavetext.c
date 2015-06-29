/*

	 wavetext - a very affordable POV Display

	 10 LEDs
	 1 ATtiny2313
	 some resistors

	 Clock: 8MHz (internal)

*/

#include <avr/io.h>
//#include <stdio.h>
//#include <stdlib.h>

#define F_CPU 8000000UL  // 8 MHz

#include <util/delay.h>

int main (void) {
	int state = 0;
	int count = 0;

	DDRA  = 0b00000011; // 5pins not accessible,Reset,LED- 0-1
	PORTA = 0b00000000;

  DDRB  = 0b00011111; // SCK,MISO,MOSI,LED+ 0-4 (1=Out, 0=In)
  PORTB = 0b00000000;

	DDRD  = 0b00000000; // not acessible, nc, Button, nc, 2*sensDir, 2*receive
	PORTD = 0b00101100;

  while (1==1) {
    //waitms(100);
		if !(PIND & 1<<5) {
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
  }
}

//void waitms(int ms) {
//  int i;
//  for (i=0;i<ms;i++) _delay_ms(1);
//}

