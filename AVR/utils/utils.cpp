#ifdef UNITTEST
#define F_CPU 1000000UL
#define BAUD 4800
#define UART_DELAY F_CPU/(BAUD)
#endif
#include "utils.hpp"
#include <avr/io.h>
#include <util/delay.h>

/** toByte[toNibble] |= fromByte[fromNibble] */
void OrNibble( uint8_t  fromByte, Nibble fromNibble,
               uint8_t& toByte,   Nibble toNibble )
{
    uint8_t temp = fromByte;
    if( fromNibble == lo )
    {
        temp &= 0x0F; // Zero out high nibble.
        if( toNibble == hi )
        {
            temp<<=4;
        }
    } else
    {
        temp &= 0xF0;
        if( toNibble == lo )
        {
            temp>>=4;
        }
    }
    toByte |= temp;
}

/** toByte[toNibble] &= fromByte[fromNibble] */
void AndNibble( uint8_t  fromByte, Nibble fromNibble,
                uint8_t& toByte,   Nibble toNibble )
{
    uint8_t temp = fromByte;
    if( fromNibble == lo )
    {
        temp |= 0xF0;
        if( toNibble == hi )
        {
            temp<<=4;
            temp |= 0x0F;
        }
    } else
    {
        temp |= 0x0F;
        if( toNibble == lo )
        {
            temp>>=4;
            temp |= 0xF0;
        }
    }
    toByte &= temp;
}

/** toByte[toNibble] = fromByte[fromNibble] */
void SetNibble( uint8_t  fromByte, Nibble fromNibble,
                uint8_t& toByte,   Nibble toNibble )
{
    uint8_t temp = fromByte;
    if( fromNibble == lo )
    {
        temp &= 0x0F;
        if( toNibble == lo )
        {
            OrNibble(toByte, hi, temp, hi);
        } else
        {
            temp<<=4;
            OrNibble(toByte, lo, temp, lo);
        }
    } else
    {
        temp &= 0xF0;
        if( toNibble == hi )
        {
            OrNibble(toByte, lo, temp, lo);
        } else
        {
            temp>>=4;
            OrNibble(toByte, hi, temp, hi);
        }
    }
    toByte = temp;
}


/** E.g. if c==0xAA, fills buf with "10101010".
    Buf better have 9 bytes allocated to it.
*/
void char2bits(char* buf, char c)
{
    for( int i=0; i<8; ++i )
    {
        buf[i] = ((128>>i) & c) ? '1':'0';
    }
    buf[8] = 0;
}


/** Templatized because if you pass a variable to _delay_ms(), that adds
 *  about 10K to the .hex file.
*/
template<int T> void blinkLED()
{
    DDRB |= 0x01;
    PORTB |= 0x01;
    _delay_ms(T);
    PORTB &= ~0x01;
}


void EEPROM::Write(uint16_t addr, uint8_t data) const
{
    // Wait for completion of previous write.
    while(EECR & (1<<EEPE))
        ;

    // Set Programming mode to atomic erase&write.
    EECR = (0<<EEPM1)|(0<<EEPM0);

    EEAR = addr;
    EEDR = data;

    // Trigger eeprom write.
    EECR |= (1<<EEMPE);
    EECR |= (1<<EEPE);
}


uint8_t EEPROM::Read(uint16_t addr) const
{
  while(EECR & (1<<EEPE)) // Wait for completion of previous write.
    ;
  EEAR = addr;            // Set up address register.
  EECR |= (1<<EERE);      // Start eeprom read by writing EERE.
  return EEDR;
}


#ifdef UNITTEST

int main()
{
#ifdef __AVR_ATtiny85__

    BitBangUART bbu(&PORTB, 4);
    DDRB = 0x10;
    while(1)
    {
        bbu.putch('U');
        _delay_ms(5);
/*
        bbu.putch(0xF0);
        _delay_ms(5);
        bbu.putch(0x0F);
        _delay_ms(5);
*/
    }
#endif // __AVR_ATtiny85__
}


#ifdef LINUX // FIXME: what's a better symbol?
//
// For compiling and running on Linux, not AVR.
//
#include <cstdio>
#include <cstring>
using namespace std;

int main()
{
    uint8_t from = 0x78;
    uint8_t to = 0xAD;
    Nibble nibs[] = {lo,hi};
    char buf[9];
    for( int iNib=0; iNib<=1; ++iNib )
    {
        for( int jNib=0; jNib<=1; ++jNib )
        {
            uint8_t newTo = to;
            SetNibble( from, nibs[iNib], newTo, nibs[jNib] );
            char2bits(buf, newTo);
            printf("SetNibble(0x%X, %d, 0x%X, %d): 0x%X, %s\n", 
                   from, iNib, to, jNib, newTo, buf);
        }
    }

    for( int iNib=0; iNib<=1; ++iNib )
    {
        for( int jNib=0; jNib<=1; ++jNib )
        {
            uint8_t newTo = to;
            OrNibble( from, nibs[iNib], newTo, nibs[jNib] );
            char2bits(buf, newTo);
            printf("OrNibble(0x%X, %d, 0x%X, %d): 0x%X, %s\n", 
                   from, iNib, to, jNib, newTo, buf);
        }
    }
    printf("\n");

    for( int iNib=0; iNib<=1; ++iNib )
    {
        for( int jNib=0; jNib<=1; ++jNib )
        {
            uint8_t newTo = to;
            AndNibble( from, nibs[iNib], newTo, nibs[jNib] );
            char2bits(buf, newTo);
            printf("AndNibble(0x%X, %d, 0x%X, %d): 0x%X, %s\n", 
                   from, iNib, to, jNib, newTo, buf);
        }
    }
}
#endif // LINUX

#endif // UNITTEST
