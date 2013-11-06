// Blinky.c
//
// From _C Programming for Microcontrollers_
// (extract thereof in pdf format, at http://smileymicros.com).
//

#define DELAY 50
#include <avr/io.h>
#define F_CPU 8000000UL // Something about processor speed
#include <util/delay.h>
int main (void)
{
    int i;
    // set PORTD for output
    // On atmega16, if we try portb, avrdude abends.  Using portc lets us
    // do the programming, but the runtime behavior is odd; only some of
    // the LEDs turn on, and they're sometimes dim and sometimes bright.
    // Portd is like that too, except the particular LEDs that turn on
    // are different.  Porta is better; everything works except its pin7.
    // Maybe I trashed my atmega16 during my attempt to program it on the
    // breadboard (using the STK500's ISP6PIN header).
    DDRD = 0xFF;
    while(1)
    {
        for(i = 1; i <= 128; i = i*2)
        {
            PORTD = i^0xFF;
            _delay_ms(DELAY);
        }
        for(i = 128; i > 1; i -= i/2)
        {
            PORTD = i^0xFF;
            _delay_ms(2*DELAY);
        }
    }
    return 1;
}

