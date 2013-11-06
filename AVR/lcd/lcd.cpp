#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "lcd.hpp"

// Control port and its constants.
#define CTRLPORT_DDR  DDRC
#define CTRLPORT  PORTC // Pin numbers below reflect convenient hookups using
    // the 5x2-headed ribbon cable.   And fact that PORTC6 on the atmega168 is
    // RST.
#define LCD_RS   0  // register select
#define LCD_RW   2  // read=1, write=0
#define LCD_E    4  // enable signal
#define LCD_BKLT 5  // backlight positive voltage

#define SPI_PORT PORTB
#define SPI_DDR  DDRB
#define SPI_SS   2
#define SPI_MOSI 3
#define SPI_SCK  5

// LCD memory that maps to positions in the LCD display.
#define DDRAM_ROW0 0x80
#define DDRAM_ROW1 0xC0


void InitSPI()
{
    // Set MOSI and SCK output, all others input.
    SPI_DDR |= (1<<SPI_MOSI) | (1<<SPI_SCK) | (1<<SPI_SS);
    // Enable SPI, Master, set clock rate fck/16 */
    SPCR |= (1<<SPE) | (1<<MSTR) | (1<<SPR0);
}


void
LCD::SetBacklight(bool b)
{
    if( b ) CTRLPORT |= _BV(LCD_BKLT);
    else    CTRLPORT &= ~_BV(LCD_BKLT);
    _backlight = b;
}

/** Pulse the enable bit low (to send data or command to LCD).
*/
void 
LCD::PulseE() const
{
    _delay_ms(1);  // Data inputs have to be stable for at least 200ns.
    CTRLPORT &= ~_BV(LCD_E);
    _delay_ms(1);  // E has to be low for at least 500ns.
    CTRLPORT |= _BV(LCD_E);
}


void 
LCD::SendData(uint8_t d) const
{
    WhileBusy();
    SPI_PORT &= ~(1<<SPI_SS);

    SPDR = d;
    while( bit_is_clear(SPSR,SPIF) ); // Wait for end of xmission.
    SPI_PORT |= (1<<SPI_SS);  // Shift reg will now move data to its pins.

    PulseE();
}    


void
LCD::SendCharacter(uint8_t d) const
{
    CTRLPORT |= _BV(LCD_RS); // data mode
    SendData(d);
}    


void
LCD::SendCommand(uint8_t d) const
{
    CTRLPORT &= ~_BV(LCD_RS); // command mode
    SendData(d);
}    


LCD::LCD()
{
    InitSPI();

    CTRLPORT_DDR = 0xFF;
    CTRLPORT = _BV(LCD_E);
    // Enable bit should be high at all times, except when we want to pulse it

    WhileBusy();

    // 8-bit data interface, 2-line display, 5x7 pixel characters.
    SendCommand(0x38); // 00111000

    Clear();
    CursorHome();

    // Display on, non-blinking cursor.
    SendCommand(0x0E);

    SetBacklight(true);
}


/** Poll the busy flag -- bit 7. */
void
LCD::WhileBusy() const
{
    // We'll just delay a little.  Actually checking the LCD's busy flag means
    // reading from the shift register, over SPI.
    // (See version 1.11 for how to check the busy flag.)
    _delay_ms(1);
}


void
LCD::CursorHome() const
{
    SendCommand(0x02);
}


void
LCD::Clear(int row) const
{
//  SendCommand(0x01) unfortunately clears both rows, so we're forced to do
//  something slower:
    CursorHome();
    CursorMove(row,0);
    for(int i=0;i<16;++i)
    {
        SendCharacter(' ');
    }
}


void
LCD::CursorMove(int row, int col) const
{
    // This approach doesn't work if you want to check the busy flag in
    // LCD::WhileBusy(), because after setting the DDRAM address the busy flag
    // never resets to 0.  But with WhileBusy() now taking the simple approach
    // of sleeping for a millisecond, this here technique works.   See version
    // 1.11 for the other way of doing it.
    if     ( row == 0 ) SendCommand( DDRAM_ROW0 + col );
    else if( row == 1 ) SendCommand( DDRAM_ROW1 + col );
}


void
LCD::Puts(char const* msg, int row) const
{
    Clear(row);
    CursorHome();

    CursorMove(row,0);

    for( char const* c=msg; *c; ++c )
    {
        SendCharacter(*c);
    }

}


#ifdef UNITTEST
int main(void) {
    LCD lcd;

    lcd.Puts("Kuni Lemel and", 0);
    lcd.Puts("Marilyn Jones", 1);

    for(;;);
}
#endif
