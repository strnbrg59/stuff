#define F_CPU 8000000UL

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "../uart/uart.hpp"
UART g_uart(38400L);

#ifndef min
#define min(a,b) (a) < (b) ? (a) : (b)
#endif


#define OVERALL_BYTES 800   // atmega168 has 1K SRAM
volatile uint8_t g_nChannels;
volatile int g_batchSize;
volatile int g_voltagesRecorded = 0;
volatile uint16_t g_delay;
uint8_t g_voltages[OVERALL_BYTES];

// Having found that conversions take place at very regular intervals, we'll
// dispense with the timing byte and record just the voltages, with timing
// information output only at the end of each batch.
// Output format:
//   1st: F_CPU/1E6
//   2nd: number of channels.
//   3rd&4th: g_batchSize (littlendian).
//   Subsequent bytes:  for each batch,
//      8-bit voltages cycling through all the channels.
//      Last two bytes of a batch: number of clock ticks elapsed during this
//      batch (littleendian).  Clock prescaled x 256.
//
// ADC conversions begin upon receipt of "start" over the uart, followed
// immediately by a uint8_t indicating the number of channels and a uint16_t
// indicating the argument to _delay_loop_2().


uint8_t g_tmrOverflows;

void
adcInit(void)
{
    // Left-justify result, so we can pick up the 8 most significant bits
    // right out of ADCH.
    ADMUX |= _BV(ADLAR);

    // Enable ADC.
    ADCSRA |= _BV(ADEN);
}


void timerInit()
{
    // Prescale x 256.
    TCCR0B |= _BV(2);
    TCCR0B &= ~(_BV(1) | _BV(0));

    SREG |= _BV(SREG_I);     // General interrupt enable
    TIMSK0 |= _BV(TOIE0);    // Timer overflow interrupt

    g_tmrOverflows = 0;
    TCNT0 = 0x0;
}


ISR(TIMER0_OVF_vect)
{
    ++g_tmrOverflows;
}


/** Write data header over uart. */
void outputHeader()
{
#ifdef NDEBUG
    g_uart.putch(F_CPU/1000000UL);
    g_uart.putch(g_nChannels);
    g_uart.putch(g_batchSize%256);
    g_uart.putch(g_batchSize/256);
#else
    g_uart.puts("numchannels=");
    g_uart.char2bin(g_nChannels);  g_uart.puts("\n");
#endif
}


void setAdcChannel(uint8_t channel)
{
    ADMUX = (ADMUX & 0xF0) | channel;     // atmega168 has six channels.

    // The readings are garbage if we don't delay here.  We're supposed to
    // delay one ADC cycle, but I don't know how much that is.  Anyway, it's
    // 1 microsecond seems to work and it's a negligible overhead.
    _delay_us(1);
}


void dumpData()
{
    // This prevents dumpData() interrupting itself, if it's been called from
    // main() and then it gets called from doStateTransition().
    volatile static bool s_dumpingData = false;

    if( s_dumpingData ) return;
    s_dumpingData = true;

    if( g_voltagesRecorded == 0 ) return;

    // That happens if we just exited dumpData() where it's called in main(),
    // and we got into it again from doStateTransition().

    g_uart.putch(TCNT0);
    g_uart.putch(g_tmrOverflows);
    g_tmrOverflows = TCNT0 = 0;

    for( int i=0;i<g_batchSize;++i )
    {
        for( int c=0;c<g_nChannels;++c )
        {
            // If ADC ended before the end of a batch, then fill out the
            // rest of the batch with voltage=0.
            if( i*g_nChannels + c >= g_voltagesRecorded )
            {
                g_uart.putch(0);
            } else
            {
                g_uart.putch(g_voltages[i*g_nChannels+c]);
            }
        }
    }
    g_uart.putch(TCNT0);
    g_uart.putch(g_tmrOverflows);
    s_dumpingData = false;
}


volatile char g_uartCmd[8] = "";
enum State {undefined, init, running, stopped, paused};
volatile State g_State = init;
volatile State g_State_1 = undefined; // prev state

void doStateTransition()
{
    if( !strcmp((char*)g_uartCmd, "start") )
    {
        //
        // "start" has to be followed by:
        // 1. Number of channels (uint8_t)
        // 2. delay_power ( = log_2(g_delay) )
        //
        g_nChannels = g_uart.getch();
        g_batchSize = OVERALL_BYTES/g_nChannels;

        uint8_t delay_power;
        delay_power = g_uart.getch();
        delay_power = min(delay_power, 16);
        g_delay = 1;
        for( int i=0;i<delay_power;++i )
        {
            g_delay *= 2;
        }
        if( g_delay == 0 ) g_delay -= 1; // 256*256-1

#ifndef NDEBUG
        g_uart.puts("channels="); g_uart.char2bin(g_nChannels);
        g_uart.puts("delay_power="); g_uart.char2bin(delay_power);
        g_uart.puts("\n");
#endif
        adcInit();
        timerInit();
        outputHeader();
        g_State_1 = g_State;
        g_State = running;
    } else
    if( !strcmp((char*)g_uartCmd, "stop") )
    {
        g_State_1 = g_State;
        g_State = stopped;
        dumpData();
    }
}


ISR(USART_RX_vect)
{
    uint8_t err;
    g_uart.gets((char*)g_uartCmd, 8, err);
    doStateTransition();
    g_uart.flushInput();
}


/** Optimization.  Making the limit of the inner loop a constant speeds things
 *  up by about 50%.  On an 8MHz atmega168V, that means we can sample at 122KHz
 *  versus 84KHz.
*/
template<int C, int N> struct AdcInnerLoop
{
    void operator()()
    {
        setAdcChannel(N);
        ADCSRA |= _BV(ADSC);  // Start ADC conversion
        while( bit_is_set(ADCSRA,ADSC) ) ;
        g_voltages[g_voltagesRecorded++] = ADCH;

        AdcInnerLoop<C,N+1>()();
    }
};

template<int C> struct AdcInnerLoop<C,C>
{
    void operator()() { return; }
};


template<int C> void adcLoop()
{
    for( int i=0;i<OVERALL_BYTES/C;++i )
    {
        AdcInnerLoop<C,0>()();
        // Delay here to reduce sampling rate.
        _delay_loop_2(g_delay);
        // Arg to _delay_us has to be a constant, or else this function adds
        // about 10K to the .hex file.
    }
} 


int
main()
{
    while(1)
    {
        while( g_State != running );

#ifdef NDEBUG
        // Do g_batchSize conversions, per channel, and store.  When done with
        // that, send the data followed by timing information (as per
        // description of output format, above).
        // 
        g_tmrOverflows = TCNT0 = 0;

        //
        // On an 8MHz atmega168, you'll get about 84KHz ADC rate here.  But
        // if you replace g_batchSize and g_nChannels with literal constants
        // (e.g. 800 and 1) you'll get 137KHz.
        //
        g_voltagesRecorded = 0;
        switch(g_nChannels) {
            case 1: adcLoop<1>(); break;
            case 2: adcLoop<2>(); break;
            case 3: adcLoop<3>(); break;
            case 4: adcLoop<4>(); break;
            default: adcLoop<1>(); // FIXME: how to indicate an error??
        }
        dumpData();

#else
        // Just print out voltages.  No batching.
        for( int c=0;c<g_nChannels;++c )
        {
            setAdcChannel(c);
            ADCSRA |= _BV(ADSC);
            while( bit_is_set(ADCSRA,ADSC) ) ;
            g_uart.puts("channel "); g_uart.char2bin(c); g_uart.puts("=");
            g_uart.char2bin(ADCH);
            g_uart.puts(" ");
        }
        g_uart.puts("\n");
        _delay_ms(500);
#endif
    }
}
