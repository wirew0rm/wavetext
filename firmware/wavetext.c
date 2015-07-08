/*

	 wavetext - a very affordable POV Display

	 10 LEDs
	 1 ATtiny2313
	 some resistors

	 Clock: 8MHz (internal)

*/

#include <avr/io.h>
#include <avr/interrupts.h>

#define F_CPU 8000000UL  // 8 MHz

#include <util/delay.h>

#define BUTTON !(PIND & 1<<5)

const char array[10][11] EEMEM = 
{
	   "Freakquenz", "CHAOS DA", "WALDECK", "FREAKQUENZ", "TEST TEST",
		 "Waldeck", "wirew0rm", "Rock on!", "DARMSTADT", "DONT PANIC!"
};

const uint8_t array[10][60] PROGMEM;

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

int main (void) {
	int state = 0;
	int count = 0;

	DDRA  = 0b00000011; // 5pins not accessible,Reset,LED- 0-1
	PORTA = 0b00000000;

  DDRB  = 0b00011111; // SCK,MISO,MOSI,LED+ 0-4 (1=Out, 0=In)
  PORTB = 0b00000000;

	DDRD  = 0b00000000; // not acessible, nc, Button, nc, 2*sensDir, 2*receive
	PORTD = 0b00101100;

	setupTimer();
	setupInts(); // setup the interrupt routines for the wave detection

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
  }
}

//void waitms(int ms) {
//  int i;
//  for (i=0;i<ms;i++) _delay_ms(1);
//}

