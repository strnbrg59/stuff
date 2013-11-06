// INT0.c
//
// STK500 project to respond to button presses on INT0 and INT1.
// INT0 turns LED on, INT1 turns it off.
// On-LED also pulls TX on RS232 Aux high.
//
// Setup:
//   10-pin jumper from PB to LED.
//    2-pin jumper from PD0/1 to SW0/1  (INT0/1 thus control switches 0/1).
//    2-pin jumper from PD2/3 to RS232 Spare RXD/TXD.
//

#include <avr/io.h>
#define F_CPU 1000000UL /* CPU clock in Hertz */
#include <util/delay.h>

#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define LED_PORT     PORTB
#define LED          PB6
#define LED_DDR      DDRB

void ledOn(int on_off) {
    if(on_off) {
        LED_PORT = ~_BV(LED);
    } else
    {
        LED_PORT = 0xFF;
    }
}

ISR(INT0_vect)
{
    PORTD |= _BV(1);
    ledOn(1);
}
ISR(INT1_vect)
{
    PORTD &= ~_BV(1);
    ledOn(0);
}

static void
ioinit(void)
{
    LED_DDR  = 0xFF;
    LED_PORT = 0xFF;

    // INT0 setup
    DDRD = 0x00;             // INT0 is PORTD2
    MCUCSR = 0x02;           // Falling edge on INT0
    MCUCSR |= 0x08;          // Falling edge on INT1
    SREG |= _BV(SREG_I);     // General interrupt enable
    GICR |= _BV(6) | _BV(7); // Enable INT0 and INT1

    // RXD/TXD setup
    DDRD |= _BV(0);
    DDRD |= _BV(1);
}

int
main(void)
{
    ioinit();

    for (;;)
    {
    }
}
