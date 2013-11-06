//
// STK500 project to exercise UART communications.
// 

#include "uart.hpp"
#include <string.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#ifdef __AVR_ATmega168__
#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UBRRL UBRR0L
#define UBRR UBRR0
#define TXEN TXEN0
#define RXEN RXEN0
#define RXC RXC0
#define RXCIE RXCIE0
#define RXC RXC0
#define UDR UDR0
#define UDRE UDRE0
#endif // ifdef __AVR_ATmega168__

UART::UART(long int baud) :
  _origUCSRB(UCSRB),
  _origUBRRL(UBRRL),
  _origSREG(SREG),
  _baud(baud)
{
    SREG |= _BV(SREG_I);     // General interrupt enable
    UCSRB |= _BV(TXEN) | _BV(RXEN) | _BV(RXCIE);
    UBRR = (F_CPU / (16 * _baud)) - 1;
}


UART::~UART()
{
    UCSRB = _origUCSRB;
    UBRR = _origUBRRL;
    SREG = _origSREG;
}

void
UART::putch(char c) const
{
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
}

void
UART::enableInterrupt(bool b) const
{
    if(b) UCSRB |= _BV(RXCIE);
    else  UCSRB &= ~_BV(RXCIE);
}


/** Together with "serialport -r" this produces nicely formatted
 *  messages.  In minicom, however, we'd need to send a '\n' in addition
 *  to the '\r'.
*/
void
UART::puts(char const * msg) const
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


/** Sends c's binary representation out over the uart.
 */
void
UART::char2bin(char c) const
{
    for( int i=0; i<8; ++i )
    {
        putch(((128>>i) & c) ? '1':'0');
    }
    putch(0);
}


char
UART::getch() const
{
    loop_until_bit_is_set(UCSRA,RXC);
    return UDR;
}


void
UART::gets(char* buf, int maxbuflen, uint8_t& err) const
{
    // Expects 0-terminated string over uart.  Stores whole thing, including
    // the 0, in buf.
    // If doesn't find the 0, sets err to 1.
    int pos=0;
    while( (buf[pos++] = getch())
    &&     (pos < maxbuflen) );
    if( pos == maxbuflen ) err = 1;
    else                   err = 0;
}


void
UART::flushInput() const
{
    volatile uint8_t dummy;
    while(bit_is_set(UCSRA,RXC)) dummy = UDR;
}


#ifdef UNITTEST
// Build like this: "make DEFS=-DUNITTEST burn".

UART g_uart(38400L);

ISR(USART_RX_vect)
{
#define BUFLEN 8
    char buf[BUFLEN];
    uint8_t err;
    g_uart.gets(buf, BUFLEN, err);
    if( err )
    {
        for(int i=0;i<10;++i)
        {
            PORTC=0xEF; _delay_ms(100);
            PORTC=0xDF; _delay_ms(100);
        }
        PORTC=0xFF;
    }

    if(      !strcmp(buf, "led4") ) PORTC = 0xEF;
    else if( !strcmp(buf, "led5") ) PORTC = 0xDF;
}

int main(void) {
    // PORTC for signaling that RX has seen this or that.
    DDRC = 0xFF;
    PORTC = 0xFF;

    for (;;) {
        _delay_ms(1000);
        g_uart.puts("Kuni Lemel.\n");
        _delay_ms(1000);
        g_uart.puts("Muni Lemel.\n");
        _delay_ms(1000);
        g_uart.puts("Marilyn Jones.\n");
    }
}
#endif // UNITTEST
