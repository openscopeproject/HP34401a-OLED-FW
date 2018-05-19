/**
 * This file contains all definitions for OSP lcd library.
 */

/* Define to prevent recursive inclusion */
#ifndef __LCD_H
#define __LCD_H

#include "Adafruit_GFX.h"

/* Color definitions */
#define LCD_BLACK 0x0000
#define LCD_BLUE 0x1F00
#define LCD_RED 0x00F8
#define LCD_GREEN 0xE007
#define LCD_CYAN 0xFF07
#define LCD_MAGENTA 0x1FF8
#define LCD_YELLOW 0xE0FF
#define LCD_WHITE 0xFFFF
#define LCD_GREY 0xc339

#define LCD_RD_PORT GPIOB
#define LCD_RD_PIN 5
#define LCD_WR_PORT GPIOB
#define LCD_WR_PIN 6
#define LCD_RS_PORT GPIOB
#define LCD_RS_PIN 7
#define LCD_CS_PORT GPIOB
#define LCD_CS_PIN 8
#define LCD_RST_PORT GPIOB
#define LCD_RST_PIN 9

#define PIN_LOW(port, pin) ((port)->regs->BRR = 1 << (pin))
#define PIN_HIGH(port, pin) ((port)->regs->BSRR = 1 << (pin))

#define RD_LOW PIN_LOW(LCD_RD_PORT, LCD_RD_PIN)
#define RD_HIGH PIN_HIGH(LCD_RD_PORT, LCD_RD_PIN)
#define WR_LOW PIN_LOW(LCD_WR_PORT, LCD_WR_PIN)
#define WR_HIGH PIN_HIGH(LCD_WR_PORT, LCD_WR_PIN)
#define CD_COMMAND PIN_LOW(LCD_RS_PORT, LCD_RS_PIN)
#define CD_DATA PIN_HIGH(LCD_RS_PORT, LCD_RS_PIN)
#define CS_ACTIVE PIN_LOW(LCD_CS_PORT, LCD_CS_PIN)
#define CS_IDLE PIN_HIGH(LCD_CS_PORT, LCD_CS_PIN)
#define RESET_ACTIVE PIN_LOW(LCD_RST_PORT, LCD_RST_PIN)
#define RESET_IDLE PIN_HIGH(LCD_RST_PORT, LCD_RST_PIN)

#define write_8(d)                                                             \
  { GPIOA->regs->BSRR = (0xFF << 16) | (d); }
#define read_8() (GPIOA->regs->IDR & 0xFF)

#define SET_WRITE_DIRECTION()                                                  \
  { GPIOA->regs->CRL = 0x33333333; }
#define SET_READ_DIRECTION()                                                   \
  { GPIOA->regs->CRL = 0x44444444; }

#define WR_STROBE                                                              \
  {                                                                            \
    WR_LOW;                                                                    \
    WR_HIGH;                                                                   \
  }
#define RD_STROBE                                                              \
  { RD_HIGH, RD_LOW, RD_LOW, RD_LOW; }

#define write8(x)                                                              \
  {                                                                            \
    write_8(x);                                                                \
    WR_STROBE;                                                                 \
  }

#define read8(dst)                                                             \
  {                                                                            \
    RD_STROBE;                                                                 \
    dst = read_8();                                                            \
    RD_HIGH;                                                                   \
  }
#define read16(dst)                                                            \
  {                                                                            \
    uint8_t hi;                                                                \
    read8(hi);                                                                 \
    read8(dst);                                                                \
    dst |= (hi << 8);                                                          \
  }

#define WriteCmd(x)                                                            \
  {                                                                            \
    CD_COMMAND;                                                                \
    write8(x);                                                                 \
  }
#define WriteData(x)                                                           \
  {                                                                            \
    CD_DATA;                                                                   \
    uint8_t h = (x) >> 8, l = x;                                               \
    write8(h);                                                                 \
    write8(l);                                                                 \
  }

class LCD : public Adafruit_GFX {
public:
  LCD();
  void reset(void);
  void begin(uint16_t ID);
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
  }
  uint16_t readID(void);
  void WriteCmdData(uint8_t cmd, uint16_t dat);
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                        uint16_t color);
  virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    fillRect(x, y, 1, h, color);
  }
  virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    fillRect(x, y, w, 1, color);
  }
  virtual void fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
  }
  virtual void setRotation(uint8_t r);
  virtual void invertDisplay(bool i);
  int16_t readGRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h);
  uint16_t readPixel(int16_t x, int16_t y) {
    uint16_t color;
    readGRAM(x, y, &color, 1, 1);
    return color;
  }
  void setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1);
  void vertScroll(int16_t top, int16_t scrollines, int16_t offset);
  virtual void startWrite(void);
  virtual void endWrite(void);

protected:
  uint32_t readReg32(uint16_t reg);
  uint32_t readReg40(uint16_t reg);

private:
  uint16_t _lcd_ID, _lcd_rev, _lcd_madctl, _lcd_drivOut, _MC, _MP, _MW, _SC,
      _EC, _SP, _EP;
};

#endif /* __LCD_H */
