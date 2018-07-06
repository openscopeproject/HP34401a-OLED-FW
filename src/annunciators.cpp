#include "annunciators.h"
#include "config.h"
#include "display.h"

extern Display display;

namespace Annunciators {

uint16_t last_state;

const char annunciators_text[][6] = {"  *",  "Adrs",  " Rmt",  " Man", "Trig",
                                     "Hold", " Mem",  "Ratio", "Math", "ERROR",
                                     "Rear", "Shift", "DIO",   "CON",  "4W"};

void update(uint16_t state) {
  uint16_t diff = (last_state ^ state);
  uint16_t tstate = state;
  display.setTextSize(GLYPH_FONT_SIZE);
  display.setTextColor(LCD_LIGHT_GRAY, LCD_BLACK);
  for (int i = 0; i < 11; i++) {
    if (diff & 1) {
      display.setCursor(X_OFFSET + i * X_MULT, Y_OFFSET);
      if (tstate & 1) {
        display.print(annunciators_text[i]);
      } else {
        display.print("     ");
      }
    }
    diff >>= 1;
    tstate >>= 1;
  }
  // skip over shift
  diff >>= 1;
  tstate >>= 1;
  for (int i = 12; i < 15; i++) {
    if (diff & 1) {
      display.setCursor(GLYPH_X_OFFSET,
                        GLYPH_Y_OFFSET - (i - 12) * GLYPH_Y_MULT);
      if (tstate & 1) {
        display.print(annunciators_text[i]);
      } else {
        display.print("   ");
      }
    }
    diff >>= 1;
    tstate >>= 1;
  }
  last_state = (state & 0xF7FF) | (last_state & 0x800);
}

void toggleShift() {
  display.setTextSize(GLYPH_FONT_SIZE);
  display.setTextColor(LCD_LIGHT_GRAY, LCD_BLACK);
  display.setCursor(X_OFFSET + 11 * X_MULT, Y_OFFSET);
  if (last_state & (1 << 11)) {
    display.print("     ");
  } else {
    display.print(annunciators_text[11]);
  }
  last_state ^= 0x800;
}

void clearShift() {
  if (last_state & 0x800) {
    toggleShift();
  }
}

} // namespace
