#ifndef _LCD_HPP_
#define _LCD_HPP_

#include <stdint.h>

class LCD
{
  public:
    LCD();
    void Puts(char const* msg, int row) const;
    void CursorMove(int row, int col) const;
    void SetBacklight(bool b);
    bool GetBacklight() const { return _backlight; }
    void CursorHome() const;
    void Clear(int row=0) const;

  private:
    void PulseE() const;
    void SendCommand(uint8_t d) const;
    void SendData(uint8_t d) const;
    void SendCharacter(uint8_t d) const;
    void WhileBusy() const;

    bool _backlight;
};

#endif
