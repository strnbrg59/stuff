#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "../uart/uart.hpp"
UART g_uart(38400L);

int main()
{
    DDRB &= ~_BV(0);

    while(1)
    {
        if(bit_is_set(PINB,0))
        {
            g_uart.puts("hello\n");
        }
        _delay_ms(200);
    }
}
