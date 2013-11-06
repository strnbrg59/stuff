//
// Check default values of UCSR{A,B,C} registers.
// Datasheet says they should be 00100000, 0x0, and
// 10000110 respectively.
//

#include <avr/io.h>
#define F_CPU 8000000UL /* CPU clock in Hertz */
#include <avr/pgmspace.h>

#define LED_PORT     PORTB
#define LED_DDR      DDRB

static void
ioinit(void)
{
    LED_DDR  = 0xFF;
    LED_PORT = 0xFF;
}

int
main(void)
{
    ioinit();
    UCSRB |= _BV(TXEN);

    // See p.154 of datasheet, for why reading UCSRC needs to follow
    // this hackish pattern.  (Reading UCSRA and UCSRB is normal, though.)
    unsigned char ucsrc;
    ucsrc = UCSRC;
    ucsrc = UCSRC;

    for (;;)
    {
        LED_PORT = ~UCSRB;
    }
}
