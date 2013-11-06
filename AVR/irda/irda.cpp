#define F_CPU 8000000UL /* CPU clock in Hertz */
#include <util/delay.h>

#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#ifdef USE_STDIO
#include <stdio.h>
char g_intbuf[4];
#endif
#include "../lcd/lcd.hpp"
#include "../utils/utils.hpp"

//
// The uart is needed only for debugging.  It's convenient to go uart-less
// if you need the STK500's RX/TX pins for the oscilloscope.  (If you compile
// irda.cpp with USE_UART, then its RX pin will expect to be connected (if only
// to ground).)
//
#ifndef USE_UART
struct UART { // Dummy UART struct
    UART(int) {}
    void puts(char const *) {}
    void char2bin(char) {}
};
#else
#include "../uart/uart.hpp"
#endif

UART g_uart(38400L);
LCD g_lcd;

volatile uint8_t g_nPulses=0;
volatile uint8_t g_nOverflows=0;
volatile bool g_seen96Bursts=false;
volatile uint8_t g_currBurst=0;
volatile bool g_waitingFor96=true;
#define MAX_BURSTS 20
volatile uint8_t g_burstLengths[MAX_BURSTS];

void Display(char const * msg)
{
    g_uart.puts(msg);
    g_uart.puts("\n");
    g_lcd.Puts(msg,0);
}


ISR(ANALOG_COMP_vect)
{
    TCNT0 = 0;
    ++g_nPulses;
}


ISR(TIMER0_OVF_vect)
{
    // After counting 96 pulses, print out the pulse lengths since the
    // last 96, or accumulate the next set of bursts until the next 96.
    if(g_waitingFor96)
    {
        if(g_nPulses == 96)
        {
            g_waitingFor96 = false;
        }
    } else
    {
        if(g_nPulses != 96)  // Should be 24 or 48.
        {
            if((g_currBurst < MAX_BURSTS) && (g_nPulses > 0))
            {
                g_burstLengths[g_currBurst++] = g_nPulses;
            }
        } else               // Report what's in g_burstLengths.
        {
#ifdef DEBUG
            for( int i=0;i<g_currBurst;++i )
            {
                g_uart.puts("Burst of ");
#ifdef USE_STDIO
                snprintf(g_intbuf, 4, "%d", g_burstLengths[i]);
                g_uart.puts(g_intbuf);
#else
                g_uart.char2bin(g_burstLengths[i]);
#endif
                g_uart.puts("\n");
            }
#endif // DEBUG

            // Display as a binary number with 24=0 and 48=1.
            uint8_t bincode = 0;
            if( g_currBurst >= 12 )
            {
                for(int b=0;b<8;++b)
                {
                    if(g_burstLengths[b] == 48)
                    {
                        bincode |= (1<<b);
                    }
                }
#ifdef DEBUG
                g_uart.puts("First 8 bursts: ");
                g_uart.char2bin(bincode);
                g_uart.puts("\n");
#endif
                char buf9[9];
                char numbers[10][2] = {"1", "2", "3", "4", "5",
                                      "6", "7", "8", "9", "0"};
                switch(bincode)
                {
                    // Sony remote in DVD mode.
                    case(0x79) : Display("Up arrow"); break;
                    case(0x7A) : Display("Dn arrow"); break;
                    case(0x7B) : Display("Lf arrow"); break;
                    case(0x7C) : Display("Rt arrow"); break;

                    case(0x0B) : Display("Enter"); break;
                    case(0x32) : Display("Play"); break;
                    case(0x39) : Display("Pause"); break;
                    case(0x5C) : Display("Replay"); break;
                    case(0x1B) : Display("Menu"); break;
                    case(0x15) : g_uart.puts("Power\n");
                                 g_lcd.Clear();
                                 g_lcd.SetBacklight(!g_lcd.GetBacklight());
                                 break;
                    case(0x16) : Display("Open/close"); break;
                    case(0x92) : Display("Volume up"); break;
                    case(0x93) : Display("Volume down"); break;

                    default: if((0<=bincode)&&(bincode<=9))
                             {
                                Display(numbers[bincode]); break;
                             } else
                             {
                                char2bits(buf9,bincode); Display(buf9); break;
                             }
                }
            }

#ifdef DEBUG
            g_uart.puts("----- ");
#ifdef USE_STDIO
            snprintf(g_intbuf, 4, "%d", g_currBurst);
            g_uart.puts(g_intbuf);
#else
            g_uart.char2bin(g_currBurst);
#endif
            g_uart.puts(" --------\n");
#endif // DEBUG

            g_waitingFor96 = true;
            g_currBurst = 0;
            _delay_ms(250); // "debouncing" sort of
        }
    }
    g_nPulses = 0;
    TCNT0 = 0;
}


void ComparatorInit()
{
    SREG |= _BV(SREG_I);     // General interrupt enable
    ACSR |= _BV(ACIE);       // Comparator interrupt enable

    // AIN1 can't be GND; it has to be a skosh higher than that.

    ACSR |= _BV(ACIS0) | _BV(ACIS1); // Rising edge

    DIDR1 |= _BV(AIN0D) | _BV(AIN1D); // Disable digital mode on AIN pins.
    ACSR &= ~_BV(ACO);
}


void TimerInit()
{
/*
    // No prescaling.
    TCCR0B &= ~(_BV(2) | _BV(1));
    TCCR0B |= _BV(0);
*/

    // Prescale x8.  With no prescaling, Sony pulses are correctly recognized
    // with -DDEBUG but not otherwise.  Don't know why.
    TCCR0B &= ~(_BV(2) | _BV(0));
    TCCR0B |= _BV(1);

    TCNT0 = 0x0;

    // Enable overflow interrupt.
    TIMSK0 |= _BV(TOIE0);
}


int
main(void)
{
    ComparatorInit();
    TimerInit();

    for(;;);
}
