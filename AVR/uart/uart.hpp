#ifndef __UART_HPP_
#define __UART_HPP_

#include <stdint.h>

class UART
{
public:
    UART(long int baud=4800L);
    ~UART();
    void char2bin(char c) const;
    void putch(char c) const;
    void puts(char const * msg) const;
    char getch() const;
    void gets(char* buf, int maxbuflen, uint8_t& err) const;
    void flushInput() const;
    void enableInterrupt(bool) const;
private:
    uint8_t  _origUCSRB;
    uint8_t  _origUBRRL;
    uint8_t  _origSREG;
    long int _baud;
};

#endif
