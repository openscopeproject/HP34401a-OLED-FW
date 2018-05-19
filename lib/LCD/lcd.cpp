#include "lcd.h"

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

// _lcd_capable = AUTO_READINC | MIPI_DCS_REV1 | MV_AXIS | READ_BGR;

static uint16_t readReg(uint16_t reg, int8_t index) {
  uint16_t ret;
  CS_ACTIVE;
  WriteCmd(reg);
  SET_READ_DIRECTION();
  CD_DATA;
  delay(1);
  do {
    read16(ret);
  } while (--index >= 0); // need to test with SSD1963
  RD_HIGH;
  CS_IDLE;
  SET_WRITE_DIRECTION();
  return ret;
}

static uint32_t readReg32(uint16_t reg) {
  uint16_t h = readReg(reg, 0);
  uint16_t l = readReg(reg, 1);
  return ((uint32_t) h << 16) | (l);
}

static uint32_t readReg40(uint16_t reg) {
  uint16_t h = readReg(reg, 0);
  uint16_t m = readReg(reg, 1);
  uint16_t l = readReg(reg, 2);
  return ((uint32_t) h << 24) | (m << 8) | (l >> 8);
}

LCD::LCD() :
    Adafruit_GFX(320, 480) {
}

void LCD::startWrite() {
  CS_ACTIVE;
}

void LCD::endWrite() {
  CS_IDLE;
}

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

uint16_t LCD::readID(void) {
  uint16_t ret;
  uint8_t msb;
  ret = ::readReg32(0); // forces a reset() if called before begin()
  if (ret == 0x5408)    // the SPFD5408 fails the 0xD3D3 test.
    return 0x5408;
  if (ret == 0x5420) // the SPFD5420 fails the 0xD3D3 test.
    return 0x5420;
  if (ret == 0x8989) // SSD1289 is always 8989
    return 0x1289;
  ret = ::readReg32(0x67); // HX8347-A
  if (ret == 0x4747)
    return 0x8347;
  //#if defined(SUPPORT_1963) && USING_16BIT_BUS
  ret = ::readReg32(0xA1); // SSD1963: [01 57 61 01]
  if (ret == 0x6101)
    return 0x1963;
  if (ret == 0xFFFF) // R61526: [xx FF FF FF]
    return 0x1526;   // subsequent begin() enables Command Access
  //    if (ret == 0xFF00)          //R61520: [xx FF FF 00]
  //        return 0x1520;          //subsequent begin() enables Command Access
  //#endif
  ret = ::readReg40(0xBF);
  if (ret == 0x8357) // HX8357B: [xx 01 62 83 57 FF]
    return 0x8357;
  if (ret == 0x9481) // ILI9481: [xx 02 04 94 81 FF]
    return 0x9481;
  if (ret == 0x1511) //?R61511: [xx 02 04 15 11] not tested yet
    return 0x1511;
  if (ret == 0x1520) //?R61520: [xx 01 22 15 20]
    return 0x1520;
  if (ret == 0x1526) //?R61526: [xx 01 22 15 26]
    return 0x1526;
  if (ret == 0x1581) // R61581:  [xx 01 22 15 81]
    return 0x1581;
  if (ret == 0x1400) //?RM68140:[xx FF 68 14 00] not tested yet
    return 0x6814;
  ret = ::readReg32(0xD4);
  if (ret == 0x5310) // NT35310: [xx 01 53 10]
    return 0x5310;
  ret = ::readReg40(0xEF); // ILI9327: [xx 02 04 93 27 FF]
  if (ret == 0x9327)
    return 0x9327;
  uint32_t ret32 = ::readReg32(0x04);
  msb = ret32 >> 16;
  ret = ret32;
  //    if (msb = 0x38 && ret == 0x8000) //unknown [xx 38 80 00] with D3 =
  //    0x1602
  if (msb == 0x00 && ret == 0x8000) { // HX8357-D [xx 00 80 00]
    return 0x9090;                    // BIG CHANGE: HX8357-D was 0x8357
  }
  //    if (msb == 0xFF && ret == 0xFFFF) //R61526 [xx FF FF FF]
  //        return 0x1526;          //subsequent begin() enables Command Access
  if (ret == 0x1526) // R61526 [xx 06 15 26] if I have written NVM
    return 0x1526;   // subsequent begin() enables Command Access
  if (ret == 0x8552) // ST7789V: [xx 85 85 52]
    return 0x7789;
  if (ret == 0xAC11) //?unknown [xx 61 AC 11]
    return 0xAC11;
  ret = ::readReg32(0xD3); // for ILI9488, 9486, 9340, 9341
  msb = ret >> 8;
  if (msb == 0x93 || msb == 0x94 || msb == 0x98 || msb == 0x77 || msb == 0x16)
    return ret; // 0x9488, 9486, 9340, 9341, 7796
  if (ret == 0x00D3 || ret == 0xD3D3)
    return ret; // 16-bit write-only bus
  /*
   msb = 0x12;                 //read 3rd,4th byte.  does not work in
   parallel
   pushCommand(0xD9, &msb, 1);
   ret2 = readReg(0xD3);
   msb = 0x13;
   pushCommand(0xD9, &msb, 1);
   ret = (ret2 << 8) | readReg(0xD3);
   //	if (ret2 == 0x93)
   return ret2;
   */
  return ::readReg32(0); // 0154, 7783, 9320, 9325, 9335, B505, B509
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

void LCD::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // MCUFRIEND just plots at edge if you try to write outside of the box:
  if (x < 0 || y < 0 || x >= width() || y >= height())
    return;
  WriteCmdParam4(_MC, x >> 8, x, x >> 8, x);
  WriteCmdParam4(_MP, y >> 8, y, y >> 8, y);
  WriteCmdData(_MW, color);
}

void LCD::setRotation(uint8_t r) {
  uint8_t val;
  rotation = r & 3; // just perform the operation ourselves on the protected variables
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
    val |= 0x02;                       // SS
  if (_lcd_ID == 0x9481 || _lcd_ID == 0x1581)
    val &= ~0xD0;
  //            val &= (_lcd_ID == 0x1963) ? ~0xC0 : ~0xD0; //MY=0, MX=0
  //            with ML=0 for ILI9481
  _MC = 0x2A, _MP = 0x2B, _MW = 0x2C, _SC = 0x2A, _EC = 0x2A, _SP = 0x2B, _EP =
      0x2B;
  CS_ACTIVE;
  WriteCmdParamN(0x36, 1, &val);
  setAddrWindow(0, 0, width() - 1, height() - 1);
  vertScroll(0, HEIGHT, 0); // reset scrolling after a rotation
  CS_IDLE;
}

void LCD::setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1) {
  WriteCmdParam4(_MC, x >> 8, x, x1 >> 8, x1);
  WriteCmdParam4(_MP, y >> 8, y, y1 >> 8, y1);
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
  if (h > w) {
    end = h;
    h = w;
    w = end;
  }
  uint8_t hi = color >> 8, lo = color & 0xFF;
  CD_DATA;
  while (h--) {
    end = w;
    do {
      write8(hi);
      write8(lo);
    } while (--end);
  }
  CS_IDLE;
}

void LCD::invertDisplay(bool i) {
  CS_ACTIVE;
  WriteCmdParamN(i ? 0x21 : 0x20, 0, NULL);
  CS_IDLE;
}

void LCD::vertScroll(int16_t top, int16_t scrollines, int16_t offset) {
  int16_t bfa = HEIGHT - top - scrollines; // bottom fixed area
  int16_t vsp;
  if (offset <= -scrollines || offset >= scrollines)
    offset = 0;       // valid scroll
  vsp = top + offset; // vertical start position
  if (offset < 0)
    vsp += scrollines; // keep in unsigned range
  uint8_t d[6];    // for multi-byte parameters
  d[0] = top >> 8; // TFA
  d[1] = top;
  d[2] = scrollines >> 8; // VSA
  d[3] = scrollines;
  d[4] = bfa >> 8; // BFA
  d[5] = bfa;
  CS_ACTIVE;
  WriteCmdParamN(0x33, 6, d);
  d[0] = vsp >> 8; // VSP
  d[1] = vsp;
  WriteCmdParamN(0x37, 2, d);
  WriteCmdParamN(0x13, 0, NULL); // NORMAL i.e. disable scroll
  CS_IDLE;
}

#define TFTLCD_DELAY8 0x7F
static void init_table(const void *table, int16_t size) {
  uint8_t *p = (uint8_t *) table, dat[24]; // R61526 has GAMMA[22]
  while (size > 0) {
    uint8_t cmd = pgm_read_byte(p++);
    uint8_t len = pgm_read_byte(p++);
    if (cmd == TFTLCD_DELAY8) {
      delay(len);
      len = 0;
    } else {
      for (uint8_t i = 0; i < len; i++)
        dat[i] = pgm_read_byte(p++);
      WriteCmdParamN(cmd, len, dat);
    }
    size -= len + 2;
  }
}

void LCD::begin(uint16_t ID) {
  int16_t *p16; // so we can "write" to a const protected variable.
  const uint8_t *table8_ads = NULL;
  int16_t table_size;
  reset();
  switch (_lcd_ID = ID) {

  case 0x9481:
    static const uint8_t ILI9481_regValues[] = {
        // Atmel MaxTouch
        0xB0, 1,
        0x00,                   // unlocks E0, F0
        0xB3, 4, 0x02, 0x00, 0x00,
        0x00, // Frame Memory, interface [02 00 00
              // 00]
        0xB4, 1,
        0x00,             // Frame mode [00]
        0xD0, 3, 0x07, 0x42,
        0x18, // Set Power [00 43 18] x1.00, x6, x3
        0xD1, 3, 0x00, 0x07,
        0x10, // Set VCOM  [00 00 00] x0.72, x1.02
        0xD2, 2, 0x01,
        0x02,       // Set Power for Normal Mode [01 22]
        0xD3, 2, 0x01,
        0x02,       // Set Power for Partial Mode [01 22]
        0xD4, 2, 0x01,
        0x02,       // Set Power for Idle Mode [01 22]
        0xC0, 5, 0x10, 0x3B, 0x00, 0x02,
        0x11, // Set Panel Driving [10 3B 00
              // 02 11]
        0xC1, 3, 0x10, 0x10,
        0x88, // Display Timing Normal [10 10 88]
        0xC5, 1,
        0x03,             // Frame Rate [03]
        0xC6, 1,
        0x02,             // Interface Control [02]
        0xC8, 12, 0x00, 0x32, 0x36, 0x45, 0x06, 0x16, 0x37, 0x75, 0x77, 0x54,
        0x0C, 0x00, 0xCC, 1, 0x00, // Panel Control [00]
        };
    table8_ads = ILI9481_regValues, table_size = sizeof(ILI9481_regValues);
    p16 = (int16_t *) &HEIGHT;
    *p16 = 480;
    p16 = (int16_t *) &WIDTH;
    *p16 = 320;
    break;
  }
  if (table8_ads != NULL) {
    static const uint8_t reset_off[] = { 0x01, 0,   // Soft Reset
        TFTLCD_DELAY8, 150, // .kbv will power up with ONLY reset, sleep out,
                            // display on
        0x28, 0,   // Display Off
        0x3A, 1, 0x55, // Pixel read=565, write=565.
        };
    static const uint8_t wake_on[] = { 0x11, 0,            // Sleep Out
        TFTLCD_DELAY8, 150, 0x29, 0, // Display On
        };
    CS_ACTIVE;
    init_table(&reset_off, sizeof(reset_off));
    init_table(table8_ads, table_size); // can change PIXFMT
    init_table(&wake_on, sizeof(wake_on));
    CS_IDLE;
  }
  setRotation(0); // PORTRAIT
  invertDisplay(false);
}
