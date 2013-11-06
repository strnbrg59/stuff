#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

#define AUDIOPORT PORTB
#define AUDIOPIN 4
#define EEPROM_ERASE_PORT PINB
#define EEPROM_ERASE_PIN 0
#define AUDIODDR DDRB

#define NOP()  asm volatile ("nop\n")

int main (void)
{
    // Erase eeprom if PB0 is high.  So, if you want eeprom erased, you need to
    // power up (or reset) with your finger holding down the PB0 switch.
    AUDIODDR |= _BV(EEPROM_ERASE_PIN);
    AUDIOPORT &= ~_BV(EEPROM_ERASE_PIN); NOP();
//  AUDIOPORT |= _BV(EEPROM_ERASE_PIN); // pull-up
    AUDIODDR &= ~_BV(EEPROM_ERASE_PIN); NOP();
    AUDIODDR |= _BV(AUDIOPIN); // just debugging!!
    NOP();
    while(1)
    {
        if( bit_is_set(EEPROM_ERASE_PORT, EEPROM_ERASE_PIN) )
        {
            AUDIOPORT |= _BV(AUDIOPIN);
            _delay_ms(100);
            AUDIOPORT &= ~_BV(AUDIOPIN);
            _delay_ms(100);
        }
    }
}

