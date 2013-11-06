//
// Just vibrate the TX (PORTD1) pin, to test oscilloscope.
//

#include <avr/io.h>
#define F_CPU 1000000UL /* CPU clock in Hertz */
#include <util/delay.h>
#include <avr/pgmspace.h>

#define LED_PORT     PORTD
#define LED          PD1
#define LED_DDR      DDRD

void ledOn(int on_off) {
    if(on_off) {
        LED_PORT = ~_BV(LED);
    } else
    {
        LED_PORT = 0xFF;
    }
}

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

    for (;;)
    {
        _delay_ms(1);
        ledOn(1);
        _delay_ms(1);
        ledOn(0);
    }
}
