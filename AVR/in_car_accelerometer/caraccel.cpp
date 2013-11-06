#define F_CPU 8000000UL
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../accelerometer/accel.hpp"
#include "../lcd/lcd.hpp"
#include "../utils/utils.hpp"

#define ADC_CHANNEL 1
#define MOVING_AVERAGE_LENGTH 12
MovingAverage<MOVING_AVERAGE_LENGTH> g_ma(0x80);

LCD g_lcd;
volatile uint8_t g_quiescentAcceleration=255;
volatile uint8_t g_maxAccel=0;
volatile uint8_t g_tmrOvfs = 1;


/** ADC conversions */
ISR(TIMER0_OVF_vect)
{
    ADCSRA |= _BV(ADSC);  // Start ADC conversion
    while( bit_is_set(ADCSRA,ADSC) ) ;
    g_ma.push(ADCH);

    char buf9[9];
    if(++g_tmrOvfs == 0)
    {
        char2bits(buf9, g_ma.average());
        g_lcd.Puts(buf9,0);

        g_maxAccel = ADCH > g_maxAccel ? ADCH : g_maxAccel;
        char2bits(buf9, g_maxAccel);
        g_lcd.Puts(buf9,1);
    }
}


uint8_t establishQuiescentAcceleration()
{
    int total = 0;
    uint8_t n = 0;

    // Wait a while for the ADC readings to settle down.
    for( int i=0;i<100;++i )
    {
        _delay_ms(100/MOVING_AVERAGE_LENGTH);
        ADCSRA |= _BV(ADSC);  // Start ADC conversion
        while( bit_is_set(ADCSRA,ADSC) ) ;
        g_ma.push(ADCH);
        if( (i>10) && (i%10 == 0) )
        {
            total += g_ma.average();
            ++n;
        }
    }

    return total/n;
}


int main (void)
{
    SREG |= _BV(SREG_I);     // General interrupt enable

    //
    // Timer 0 -- ADC frequency
    //
    TIMSK0 |= _BV(TOIE0);    // Enable overflow interrupt
    TCCR0B |= _BV(CS00) | _BV(CS01);  // prescale /64
    DDRC &= ~_BV(ADC_CHANNEL); // LCD constructor set all the others to output.

    //
    // ADC
    //
    ADMUX |= _BV(ADLAR);
    ADCSRA |= _BV(ADEN);
    uint8_t channel = ADC_CHANNEL; // Unclaimed pin
    ADMUX = (ADMUX & 0xF0) | channel;

    // Establish the "quiescent" accelerometer reading and record it in eeprom.
    // After that, in the main loop, wait until you see a significantly higher
    // reading, at which time start writing to eeprom at regular intervals.
    g_quiescentAcceleration = establishQuiescentAcceleration();

    while(1)
    {
    }
}
