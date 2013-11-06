#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>
int main (void)
{
    uint8_t pin = 4;
    DDRB = _BV(pin);
    PORTB = 0xFF;
    volatile uint8_t* port = &PORTB;
    while(1)
    {
            *port &= ~_BV(pin);
            _delay_ms(500);
            *port |= _BV(pin);
            _delay_ms(100);
    }
    return 1;
}

