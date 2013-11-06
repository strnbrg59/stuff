#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>
#define OUTPORT PORTB
#define OUTPORTBIT 1

/**
 * Bit-bang uart
 * We won't inherit from class UART because it has too much atmega168-specific
 * stuff -- names of UART-related registers for example.
*/

class BitBangUART
{
  public:
    BitBangUART(long int baud);
    void putch(char c);
    void puts(char const* msg);
    double _us_delay;
};


BitBangUART::BitBangUART(long int baud)
  : _us_delay( 1.0E6/baud )
{
}


void
BitBangUART::putch(char c)
{
    OUTPORT |= _BV(OUTPORTBIT); // start bit
    _delay_us(_us_delay);
    int i;

    // Inverted logic: 1=lo, 0=hi voltage.
    for( i=0; i<8; ++i )
    {
        if ((128>>i) & c)
        {
            OUTPORT &= ~_BV(OUTPORTBIT);
        } else
        {
            OUTPORT |= _BV(OUTPORTBIT);
        }
        _delay_us(_us_delay);
    }

    OUTPORT |= _BV(OUTPORTBIT); // stop bit
    _delay_us(_us_delay);
}


void BitBangUART::puts(char const* msg)
{
    char const * c = msg;
    while(*c)
    {
        if(*c == '\n') putch('\r');
        else           putch(*c);
        c++;
    }
    putch(0);
}


int main (void)
{
    BitBangUART bbu(2400);

    //
    // Amazing.  If I don't reassign bbu.m_delay like this, then the program
    // doesn't work (I don't get a long sequence of 'U' characters).
    // It also doesn't work if I declare bbu.m_delay (or bbu itself) "volatile".
    //
    bbu._us_delay = 1E6/2400;

    DDRB = 0xFF;
    while(1)
    {
//        bbu.puts("Hello world\n");

        PORTB = 0xFF;
        _delay_us(bbu._us_delay);

        PORTB = 0x00;
        _delay_us(bbu._us_delay);
    }

    return 0;
}
