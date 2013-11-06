#ifndef _UTILS_HPP_
#define _UTILS_HPP_
#include <stdint.h>

enum Nibble { lo, hi };
void OrNibble( uint8_t  fromByte, Nibble fromNibble,
               uint8_t& toByte,   Nibble toNibble );
void AndNibble( uint8_t  fromByte, Nibble fromNibble,
                uint8_t& toByte,   Nibble toNibble );
void SetNibble( uint8_t  fromByte, Nibble fromNibble,
                uint8_t& toByte,   Nibble toNibble );

void char2bits( char* buf, char c);


class EEPROM
{
  public:
    uint8_t Read(uint16_t addr) const;
    void    Write(uint16_t addr, uint8_t data) const;
    void    Clear();

    enum { capacity
#if  defined(__AVR_ATtiny85__) || defined (__AVR_ATmega168__) \
  || defined(__AVR_ATtiny861__)
            = 512
#endif
         };
};


template<int N> class MovingAverage
{
  public:
    MovingAverage(uint8_t initVal) : _total(initVal*N)
    {
        for( int i=0;i<N;++i ) _vals[i] = initVal;
    }        
    void push(uint8_t v)
    {
        _total += v - _vals[0];
        for( int i=0;i<N-1;++i ) _vals[i] = _vals[i+1];
        _vals[N-1] = v;
    }

    uint8_t average() const { return _total/N; }

  private:
    uint8_t _vals[N];
    int _total;
};



template<int T> void blinkLED();

template<typename T> T min(T a, T b) { return (a < b ? a:b); }
template<typename T> T max(T a, T b) { return min<T>(b,a); }

#endif
