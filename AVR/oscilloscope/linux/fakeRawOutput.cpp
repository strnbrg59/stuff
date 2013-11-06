#include <cstdio>
#include <unistd.h>
#include <cmath>

// Write some typical adc output -- pairs of bytes consisting of ADCH and
// TCNT0.

int main()
{
    char buf[2];
    for( int i=0;i<1000;++i )
    {
        unsigned char voltage = 63 * (sin(i/10.0) + cos(1+i/20.0) + 2);
        unsigned char tmr = (i*30)%256;
        buf[0] = voltage;
        buf[1] = tmr;
        write(STDOUT_FILENO, buf, 2);
    }
}
