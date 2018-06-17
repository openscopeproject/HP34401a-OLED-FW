#include "lcd.h"

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#ifdef USE_SSD1322_DISPLAY

#warning "using ssd1322"

#define PANEL_OFFSET 0x1C
// 8kbyte framebuffer
uint8_t framebuffer[256 * 64 / 2] = {0};

#endif

LCD::LCD() : Adafruit_GFX(LCD_DIMENSION_X, LCD_DIMENSION_Y) {}

void LCD::startWrite() { CS_ACTIVE; }

void LCD::endWrite() { CS_IDLE; }

void LCD::reset() {
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

void LCD::WriteCmdData(uint8_t cmd, uint16_t dat) {
  WriteCmd(cmd);
  WriteData(dat);
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

static inline void WriteCmdParam4(uint8_t cmd, uint8_t d1, uint8_t d2,
                                  uint8_t d3, uint8_t d4) {
  WriteCmd(cmd);
  CD_DATA;
  write8(d1);
  write8(d2);
  write8(d3);
  write8(d4);
}

static inline void WriteCmdParam2(uint8_t cmd, uint8_t d1, uint8_t d2) {
  WriteCmd(cmd);
  CD_DATA;
  write8(d1);
  write8(d2);
}

void LCD::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // MCUFRIEND just plots at edge if you try to write outside of the box:
  if (x < 0 || y < 0 || x >= width() || y >= height())
    return;
#ifdef USE_SSD1322_DISPLAY
  uint8_t _x = PANEL_OFFSET + x / 4;
  WriteCmdParam2(_MC, _x, _x);
  WriteCmdParam2(_MP, y, y);
  uint16_t p = (256 * y + x) / 2;
  if (x & 1) {
    framebuffer[p] = (framebuffer[p] & 0xF0) | color;
  } else {
    framebuffer[p] = (framebuffer[p] & 0x0F) | (color << 4);
  }
  WriteCmdParam2(_MW, framebuffer[p & ~1], framebuffer[p | 1]);
#else
  WriteCmdParam4(_MC, x >> 8, x, x >> 8, x);
  WriteCmdParam4(_MP, y >> 8, y, y >> 8, y);
  WriteCmdData(_MW, color);
#endif
}

void LCD::setRotation(uint8_t r) {
#ifdef USE_SSD1322_DISPLAY
  // ignore rotation for now
  _MC = 0x15; // Set column address
  _MP = 0x75; // Set page address
  _MW = 0x5C; // Start write
#else
  uint8_t val;
  rotation =
      r & 3; // just perform the operation ourselves on the protected variables
  _width = (rotation & 1) ? HEIGHT : WIDTH;
  _height = (rotation & 1) ? WIDTH : HEIGHT;
  switch (rotation) {
  case 0:       // PORTRAIT:
    val = 0x48; // MY=0, MX=1, MV=0, ML=0, BGR=1
    break;
  case 1:       // LANDSCAPE: 90 degrees
    val = 0x28; // MY=0, MX=0, MV=1, ML=0, BGR=1
    break;
  case 2:       // PORTRAIT_REV: 180 degrees
    val = 0x98; // MY=1, MX=0, MV=0, ML=1, BGR=1
    break;
  case 3:       // LANDSCAPE_REV: 270 degrees
    val = 0xF8; // MY=1, MX=1, MV=1, ML=1, BGR=1
    break;
  }
  if (val & 0x80)
    val |= 0x01; // GS
  if ((val & 0x40))
    val |= 0x02; // SS
  val &= ~0xD0;
  _MC = 0x2A, _MP = 0x2B, _MW = 0x2C;
  CS_ACTIVE;
  WriteCmdParamN(0x36, 1, &val);
  setAddrWindow(0, 0, width() - 1, height() - 1);
  CS_IDLE;
#endif
}

void LCD::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
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
  WriteCmd(_MW);
  CD_DATA;
#ifdef USE_SSD1322_DISPLAY
  uint8_t c = color | (color << 4);
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
#else
  uint8_t hi = color >> 8, lo = color & 0xFF;
  if (h > w) {
    end = h;
    h = w;
    w = end;
  }
  while (h--) {
    end = w;
    do {
      write8(hi);
      write8(lo);
    } while (--end);
  }
#endif
  CS_IDLE;
}

void LCD::invertDisplay(bool i) {
  CS_ACTIVE;
#ifdef USE_SSD1322_DISPLAY
  WriteCmdParamN(i ? 0xA7 : 0xA6, 0, NULL);
#else
  WriteCmdParamN(i ? 0x21 : 0x20, 0, NULL);
#endif
  CS_IDLE;
}

void LCD::setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1) {
#ifdef USE_SSD1322_DISPLAY
  WriteCmdParam2(_MC, PANEL_OFFSET + x / 4, PANEL_OFFSET + x1 / 4);
  WriteCmdParam2(_MP, y, y1);
#else
  WriteCmdParam4(_MC, x >> 8, x, x1 >> 8, x1);
  WriteCmdParam4(_MP, y >> 8, y, y1 >> 8, y1);
#endif
}

#define TFTLCD_DELAY8 0x7F
static void init_table(const uint8_t *table, int16_t size) {
  uint16_t p = 0;
  uint8_t dat[24]; // R61526 has GAMMA[22]
  while (size > 0) {
    uint8_t cmd = table[p++];
    uint8_t len = table[p++];
    if (cmd == TFTLCD_DELAY8) {
      delay(len);
      len = 0;
    } else {
      for (uint8_t i = 0; i < len; i++)
        dat[i] = table[p++];
      WriteCmdParamN(cmd, len, dat);
    }
    size -= len + 2;
  }
}

#ifdef USE_SSD1322_DISPLAY
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
#else
// ILI9481 init commands
static const uint8_t reset_off[] = {
    0x01, 0,            // Soft Reset
    TFTLCD_DELAY8, 100, // .kbv will power up with ONLY reset, sleep out,
                        // display on
    0x28, 0,            // Display Off
    0x3A, 1, 0x55,      // Pixel read=565, write=565.
};
static const uint8_t table8_ads[] = {
    // Atmel MaxTouch
    0xB0, 1, 0x00,                   // unlocks E0, F0
    0xB3, 4, 0x02, 0x00, 0x00, 0x00, // Frame Memory, interface [02 00 00 00]
    0xB4, 1, 0x00,                   // Frame mode [00]
    0xD0, 3, 0x07, 0x42, 0x18,       // Set Power [00 43 18] x1.00, x6, x3
    0xD1, 3, 0x00, 0x07, 0x10,       // Set VCOM  [00 00 00] x0.72, x1.02
    0xD2, 2, 0x01, 0x02,             // Set Power for Normal Mode [01 22]
    0xD3, 2, 0x01, 0x02,             // Set Power for Partial Mode [01 22]
    0xD4, 2, 0x01, 0x02,             // Set Power for Idle Mode [01 22]
    0xC0, 5, 0x10, 0x3B, 0x00, 0x02, 0x11, // Set Panel Driving [10 3B 00 02
                                           // 11]
    0xC1, 3, 0x10, 0x10, 0x88,             // Display Timing Normal [10 10 88]
    0xC5, 1, 0x03,                         // Frame Rate [03]
    0xC6, 1, 0x02,                         // Interface Control [02]
    0xC8, 12, 0x00, 0x32, 0x36, 0x45, 0x06, 0x16, 0x37, 0x75, 0x77, 0x54, 0x0C,
    0x00, 0xCC, 1, 0x00, // Panel Control [00]
};
static const uint8_t wake_on[] = {
    0x11,          0,   // Sleep Out
    TFTLCD_DELAY8, 100, // 100ms
    0x29,          0,   // Display On
};
#endif

void LCD::begin() {
  int16_t *p16; // so we can "write" to a const protected variable.
  p16 = (int16_t *)&HEIGHT;
  *p16 = 480;
  p16 = (int16_t *)&WIDTH;
  *p16 = 320;
  reset();
  CS_ACTIVE;
  init_table(reset_off, sizeof(reset_off));
  init_table(table8_ads, sizeof(table8_ads)); // can change PIXFMT
  init_table(wake_on, sizeof(wake_on));
  CS_IDLE;
  setRotation(0); // PORTRAIT
  invertDisplay(false);
}
