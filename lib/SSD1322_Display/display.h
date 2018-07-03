/* Define to prevent recursive inclusion */
#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "Adafruit_GFX.h"

/* Dimensions */
#ifdef USE_SSD1322_DISPLAY
#define LCD_DIMENSION_X 256
#define LCD_DIMENSION_Y 64
#else
#define LCD_DIMENSION_X 320
#define LCD_DIMENSION_Y 480
#endif

/* Color definitions */
#ifdef USE_SSD1322_DISPLAY
// 4 bit greyscale display
#define LCD_BLACK 0x0
#define LCD_BLUE 0x6
#define LCD_RED 0x6
#define LCD_GREEN 0x6
#define LCD_CYAN 0xC
#define LCD_MAGENTA 0xC
#define LCD_YELLOW 0xC
#define LCD_WHITE 0xF
#else
// 16 bit RGB display
#define LCD_BLACK 0x0000
#define LCD_BLUE 0x001F
#define LCD_RED 0xF800
#define LCD_GREEN 0x07E0
#define LCD_CYAN 0x07FF
#define LCD_MAGENTA 0xF81F
#define LCD_YELLOW 0xFFE0
#define LCD_WHITE 0xFFFF
#endif

// Control pins
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
// Data pins are GPIOA[0-7]

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

class Display : public Adafruit_GFX {
public:
  Display();
  void reset(void);
  void begin();
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
  }
  void WriteCmdData(uint8_t cmd, uint16_t dat);
  virtual void setRotation(uint8_t r);
  virtual void invertDisplay(bool i);
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                        uint16_t color);
  virtual void fillScreen(uint16_t color) {
    fillRect(0, 0, _width, _height, color);
  }
  virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    fillRect(x, y, 1, h, color);
  }
  virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    fillRect(x, y, w, 1, color);
  }
  void setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1);
  virtual void startWrite(void);
  virtual void endWrite(void);

private:
  uint16_t _MC, _MP, _MW;
};

#endif /* __DISPLAY_H */
