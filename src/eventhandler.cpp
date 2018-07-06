#include "Arduino.h"
#include "annunciators.h"
#include "config.h"
#include "display.h"
#include "eventhandler.h"

extern Display display;
char dbg_msg[100];

namespace Eventhandler {

uint8_t printed;
bool need_reset = true;

void print(char c) {
  display.print(c);
  printed++;
#ifdef DEBUG
  Serial.print(c);
#endif
}

void messageByte(uint8_t byte) {
  if (need_reset) {
    printed = 0;
    display.setCursor(TXTX, TXTY);
    display.setTextColor(LCD_WHITE, LCD_BLACK);
    display.setTextSize(MAIN_FONT_SIZE);
    need_reset = false;
  }
  switch (byte) {
  case 0x84:
    print('.');
    break;
  case 0x86:
    print(',');
    break;
  case 0x8d:
  // special semicolon that blinks previous char?
  case 0x8c:
    print(':');
    break;
  case 0x81:
    // some control character
    break;
  case 0x00:
    // end of message
    for (int i = 0; i < 14 - printed; i++) {
      display.print(" ");
    }
    need_reset = true;
#ifdef DEBUG
    Serial.println();
#endif
    break;
  default:
    print((char)byte);
  }
}

void annunciators(uint8_t h, uint8_t l) {
#ifdef DEBUG
  sprintf(dbg_msg, "Annunciators: %02x%02x", h, l);
  Serial.println(dbg_msg);
#endif
  uint16_t state = h;
  state = (state << 8) + l;
  Annunciators::update(state);
}

void control(uint8_t h, uint8_t l) {
#ifdef DEBUG
  sprintf(dbg_msg, "Control: %02x%02x", h, l);
  Serial.println(dbg_msg);
#endif
}

void button(uint8_t h, uint8_t l) {
#ifdef DEBUG
  sprintf(dbg_msg, "Button: %02x %02x", h, l);
  Serial.println(dbg_msg);
#endif
  if (h == 0x9D && l == 0xCF) {
    Annunciators::toggleShift();
  } else if (l == 0xE9 || l == 0xBB || l == 0x7D) {
    Annunciators::clearShift();
  }
}

uint16_t last_fps;

void updateFps(uint16_t fps) {
  if (fps != last_fps) {
    last_fps = fps;
    display.setCursor(FPSX, FPSY);
    display.setTextSize(1);
    if (fps < 10)
      display.print(' ');
    display.print(fps);
  }
}
} // namespace
