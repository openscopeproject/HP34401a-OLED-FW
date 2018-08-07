#include "display.h"

#define COLUMN_ADDR_CMD 0x15 // Set column address
#define PAGE_ADDR_CMD 0x75   // Set page address
#define START_WRITE_CMD 0x5C // Start write

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
    GPIOA->regs->BSRR = (0xFF << 16) | (x);                                    \
    WR_STROBE;                                                                 \
  }

#define read8(dst)                                                             \
  {                                                                            \
    RD_STROBE;                                                                 \
    dst = GPIOA->regs->IDR & 0xFF;                                             \
    RD_HIGH;                                                                   \
  }

#define WriteCmd(x)                                                            \
  {                                                                            \
    CD_COMMAND;                                                                \
    write8(x);                                                                 \
  }

#define PANEL_OFFSET 0x1C
// 8kbyte framebuffer
uint8_t framebuffer[256 * 64 / 2] = {0};

void displaySetup() {
  // Configure control pins as outputs     76543210      76543210
  GPIOB->regs->CRL = (GPIOB->regs->CRL & 0xFFFFF000) | 0x00000333;
  //                                       54321098      54321098
  GPIOB->regs->CRH = (GPIOB->regs->CRH & 0xFFFF00FF) | 0x00003300;
}

Display::Display() : Adafruit_GFX(LCD_DIMENSION_X, LCD_DIMENSION_Y) {}

void Display::startWrite() { CS_ACTIVE; }

void Display::endWrite() { CS_IDLE; }

void Display::reset() {
  SET_WRITE_DIRECTION();
  CS_IDLE;
  RD_HIGH;
  WR_HIGH;
  RESET_IDLE;
  delay(50);
  RESET_ACTIVE;
  delay(100);
  RESET_IDLE;
  delay(100);
}

static void WriteCmdParamN(uint8_t cmd, int8_t N, uint8_t *block) {
  WriteCmd(cmd);
  CD_DATA;
  uint8_t u8;
  while (N--) {
    u8 = *block++;
    write8(u8);
  }
}

static inline void WriteCmdParam2(uint8_t cmd, uint8_t d1, uint8_t d2) {
  WriteCmd(cmd);
  CD_DATA;
  write8(d1);
  write8(d2);
}

void Display::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // MCUFRIEND just plots at edge if you try to write outside of the box:
  if (x < 0 || y < 0 || x >= width() || y >= height())
    return;
  if (rotation) {
    x = _width - x - 1;
    y = _height - y - 1;
  }
  uint8_t _x = PANEL_OFFSET + x / 4;
  WriteCmdParam2(COLUMN_ADDR_CMD, _x, _x);
  WriteCmdParam2(PAGE_ADDR_CMD, y, y);
  uint16_t p = (256 * y + x) / 2;
  if (x & 1) {
    framebuffer[p] = (framebuffer[p] & 0xF0) | color;
  } else {
    framebuffer[p] = (framebuffer[p] & 0x0F) | (color << 4);
  }
  WriteCmdParam2(START_WRITE_CMD, framebuffer[p & ~1], framebuffer[p | 1]);
}

void Display::setRotation(uint8_t r) { rotation = r; }

void Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                       uint16_t color) {
  if (rotation) {
    x = _width - x;
    y = _height - y;
    x -= w;
    y -= h;
  }
  int16_t end;
  if (w < 0) {
    w = -w;
    x -= w;
  } //+ve w
  end = x + w;
  if (x < 0)
    x = 0;
  if (end > width())
    end = width();
  w = end - x;
  if (h < 0) {
    h = -h;
    y -= h;
  } //+ve h
  end = y + h;
  if (y < 0)
    y = 0;
  if (end > height())
    end = height();
  h = end - y;

  CS_ACTIVE;
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  WriteCmd(START_WRITE_CMD);
  CD_DATA;
  uint16_t p = (256 * y + x) / 2;
  p = p & ~1;
  for (int i = 0; i < h; i++) {
    uint16_t p2 = p + 128 * i;
    for (int j = x / 4; j <= (x + w - 1) / 4; j++) {
      uint8_t k = j * 4;
      if (k >= x && k < x + w) {
        framebuffer[p2] = (framebuffer[p2] & 0x0F) | (color << 4);
      }
      k++;
      if (k >= x && k < x + w) {
        framebuffer[p2] = (framebuffer[p2] & 0xF0) | color;
      }
      write8(framebuffer[p2++]);
      k++;
      if (k >= x && k < x + w) {
        framebuffer[p2] = (framebuffer[p2] & 0x0F) | (color << 4);
      }
      k++;
      if (k >= x && k < x + w) {
        framebuffer[p2] = (framebuffer[p2] & 0xF0) | color;
      }
      write8(framebuffer[p2++]);
    }
  }
  CS_IDLE;
}

void Display::invertDisplay(bool i) {
  CS_ACTIVE;
  WriteCmdParamN(i ? 0xA7 : 0xA6, 0, NULL);
  CS_IDLE;
}

void Display::setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1) {
  WriteCmdParam2(COLUMN_ADDR_CMD, PANEL_OFFSET + x / 4, PANEL_OFFSET + x1 / 4);
  WriteCmdParam2(PAGE_ADDR_CMD, y, y1);
}

void Display::pushPixels(int16_t x, int16_t y, int16_t w, int16_t h,
                         const uint8_t *data, uint8_t c) {
  if (rotation) {
    x = _width - x;
    y = _height - y;
    x -= w;
    y -= h;
  }
  if (x + w > width() || y + h > height())
    return;
  CS_ACTIVE;
  setAddrWindow(x, y, x + w - 1, y + h - 1);
  WriteCmd(START_WRITE_CMD);
  uint16_t p = (256 * y + x) / 2;
  p = p & ~1;
  CD_DATA;
  for (int i = 0; i < h; i++) {
    uint16_t p2 = p + 128 * i;
    uint16_t p3 = ((w + 7) / 8) * i;
    for (int j = x / 4; j <= (x + w - 1) / 4; j++) {
      uint8_t k = j * 4;
      uint8_t color;
      if (k >= x && k < x + w) {
        color = (data[p3 + (k - x) / 8] & (0x80 >> ((k - x) & 7))) ? c : 0x0;
        framebuffer[p2] = (framebuffer[p2] & 0x0F) | (color << 4);
      }
      k++;
      if (k >= x && k < x + w) {
        color = (data[p3 + (k - x) / 8] & (0x80 >> ((k - x) & 7))) ? c : 0x0;
        framebuffer[p2] = (framebuffer[p2] & 0xF0) | color;
      }
      write8(framebuffer[p2++]);
      k++;
      if (k >= x && k < x + w) {
        color = (data[p3 + (k - x) / 8] & (0x80 >> ((k - x) & 7))) ? c : 0x0;
        framebuffer[p2] = (framebuffer[p2] & 0x0F) | (color << 4);
      }
      k++;
      if (k >= x && k < x + w) {
        color = (data[p3 + (k - x) / 8] & (0x80 >> ((k - x) & 7))) ? c : 0x0;
        framebuffer[p2] = (framebuffer[p2] & 0xF0) | color;
      }
      write8(framebuffer[p2++]);
    }
  }
  CS_IDLE;
}

static void init_table(const uint8_t *table, int16_t size) {
  uint16_t p = 0;
  uint8_t dat[24];
  while (size > 0) {
    uint8_t cmd = table[p++];
    uint8_t len = table[p++];
    for (uint8_t i = 0; i < len; i++)
      dat[i] = table[p++];
    WriteCmdParamN(cmd, len, dat);
    size -= len + 2;
  }
}

static const uint8_t reset_off[] = {
    0xFD, 1, 0x12, // Unlock command
    0xAE, 0,       // Display off
};
static const uint8_t table8_ads[] = {
    0xB3, 1, 0x91,       // Set clock divide ratio / oscillator frequency
    0xCA, 1, 0x3F,       // Set multiplex ratio to 1/64
    0xA2, 1, 0x00,       // Set offset
    0xA1, 1, 0x00,       // Set start line
    0xA0, 2, 0x14, 0x11, // Set remap and dual com line mode
    0xAB, 1, 0x01,       // Use external vdd,
    0xB4, 2, 0xA0, 0xFD, // Enable external vsl
    0xC1, 1, 0x9F,       // Set contrast current
    0xC7, 1, 0x0F,       // Set master contrast current control
    0xB9, 0,             // Use default linear grey table
    0xB1, 1, 0xE2,       // Set phase length
    0xD1, 2, 0x82, 0x20, // Enhance driving scheme capability
    0xBB, 1, 0x1F,       // Set pre-charge voltage
    0xB6, 1, 0x08,       // Set second pre-charge period
    0xBE, 1, 0x07,       // Set VCOMH deselect level
    0xA6, 0,             // Set normal display mode
};
static const uint8_t wake_on[] = {
    0xAF, 0, // Display on
};

void Display::begin() {
  reset();
  CS_ACTIVE;
  init_table(reset_off, sizeof(reset_off));
  init_table(table8_ads, sizeof(table8_ads)); // can change PIXFMT
  init_table(wake_on, sizeof(wake_on));
  CS_IDLE;
  invertDisplay(false);
}
