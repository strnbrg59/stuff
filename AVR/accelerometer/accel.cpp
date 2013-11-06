#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "accel.hpp"
#include "../utils/utils.hpp"

#define AUDIOPORT PORTB
#define AUDIOPIN 4
#define EEPROM_ERASE_PORT PINB
#define EEPROM_ERASE_PIN 4
#define AUDIODDR DDRB
#define ACCEL_ACTIVATION_PIN 0
#define NOP()  asm volatile ("nop\n")

#define MOVING_AVERAGE_LENGTH 12
MovingAverage<MOVING_AVERAGE_LENGTH> g_ma(0x80);


void flashLed(int n)
{
    AUDIODDR |= _BV(EEPROM_ERASE_PIN); NOP();
    for( int i=0;i<n;++i )
    {
        AUDIOPORT |= _BV(AUDIOPIN); _delay_ms(100);
        AUDIOPORT &= ~_BV(AUDIOPIN); _delay_ms(100);
    }
}


uint8_t scaleVoltage(uint8_t v, uint8_t s)
{
    return 255 - (128 + (v-128)*s);
}

EEPROM g_eeprom;
volatile int g_nextEepromAddr = 0;
volatile bool g_activateEepromWriting = false;
volatile uint8_t g_adcReadings = 0;
volatile uint8_t g_quiescentAcceleration=255;
volatile bool g_audioOn = false;

/** Toggle a pin that the audio speaker is hooked up to. */
ISR(TIMER0_COMPA_vect)
{
    TCNT0 = 0x00;
    if( g_nextEepromAddr == EEPROM::capacity ) 
    {
        return;
    }
    if( g_audioOn )   AUDIOPORT &= ~_BV(AUDIOPIN);
    else              AUDIOPORT |= _BV(AUDIOPIN);
    g_audioOn = !g_audioOn;
    OCR0A = scaleVoltage(g_ma.average(), 1);
}


ISR(TIMER1_COMPA_vect)
{
    TCNT1 = 0x00;

    ADCSRA |= _BV(ADSC);  // Start ADC conversion
    while( bit_is_set(ADCSRA,ADSC) ) ;
    g_ma.push(ADCH);
    if(ADCH > g_quiescentAcceleration + 10)
    {
        g_activateEepromWriting = true;
    }

    if( g_activateEepromWriting && 
        ((++g_adcReadings)%MOVING_AVERAGE_LENGTH == 0) )
    {
        if( g_nextEepromAddr < EEPROM::capacity )
        {
            g_eeprom.Write(g_nextEepromAddr++, g_ma.average());
        } else
        {
            activateAccelerometer(false);
        }
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


void activateAccelerometer(bool b)
{
    // Supply power to accelerometer.  PB0 (that being our EEPROM_ERASE_PIN)
    // is also the MOSI pin so when we're hooked up to the programming header,
    // it always has at least about 2.5V.  But with the programming header
    // removed, we should be able to exert effective control this way.
    AUDIODDR |= _BV(ACCEL_ACTIVATION_PIN);
    if(b)
    {
        AUDIOPORT |= _BV(ACCEL_ACTIVATION_PIN);
    } else
    {
        AUDIOPORT &= ~_BV(ACCEL_ACTIVATION_PIN);
    }
}


int main (void)
{
    // Erase eeprom if PB0 is high.  So, if you want eeprom erased, you need to
    // power up (or reset) with your finger holding down the button switch.
    // If eeprom is full, the program doesn't go anywhere *until* you press that
    // button (we don't want that first, "quiescent", value getting overwritten
    // when we turn the device back on after a rocket flight).
    // FIXME: cut power to the accelerometer and put AVR in sleep mode too,
    //        after eeprom is full.
    AUDIODDR &= ~_BV(EEPROM_ERASE_PIN); NOP();
    if( g_eeprom.Read(EEPROM::capacity-1) != 0 )
    {
        while( !bit_is_set(EEPROM_ERASE_PORT, EEPROM_ERASE_PIN) )
            ;
    }
    if( bit_is_set(EEPROM_ERASE_PORT, EEPROM_ERASE_PIN) )
    {
        for( int i=0;i<EEPROM::capacity;++i ) g_eeprom.Write(i, 0x00);
    }
    activateAccelerometer(true);

    SREG |= _BV(SREG_I);     // General interrupt enable

#ifndef NAUDIO
    //
    // Timer 0 -- audio speaker
    //
    TIMSK |= _BV(OCIE0A);    // Output compare interrupt
    TCCR0B |= _BV(CS01);     // prescale /8
    AUDIODDR = _BV(AUDIOPIN);
#endif

    //
    // Timer 1 -- adc read frequency
    //
    TIMSK |= _BV(OCIE1A);    // Output compare interrupt
    TCCR1 |= _BV(CS13);      // prescale /1024
    TCCR1 |= _BV(CS11);      // "
    TCCR1 |= _BV(CS10);      // "
    OCR1A =  4;              // Interrupt will occur 244.14 times per second, at
                             // 1MHz clock (and prescale 1024).  Divide that
                             // by the moving average length to get frequency of
                             // eeprom writes.
    //
    // ADC
    //
    ADMUX |= _BV(ADLAR);
    ADCSRA |= _BV(ADEN);
    uint8_t channel = 3; // Pin not used for programming
    ADMUX = (ADMUX & 0xF0) | channel;

    // Establish the "quiescent" accelerometer reading and record it in eeprom.
    // After that, in the main loop, wait until you see a significantly higher
    // reading, at which time start writing to eeprom at regular intervals.
    g_quiescentAcceleration = establishQuiescentAcceleration();
    g_eeprom.Write(0, g_ma.average());
    g_nextEepromAddr = 1;

    while(1)
    {
    }
    return 1;
}
