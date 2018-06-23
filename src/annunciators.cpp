#include "annunciators.h"
#include "lcd.h"
#include "config.h"

uint16_t last_state;
extern LCD tft;

const char annunciators_text[][6] = {"  *",  "Adrs",  " Rmt",  " Man", "Trig",
                                     "Hold", " Mem",  "Ratio", "Math", "ERROR",
                                     "Rear", "Shift", "DIO",   "CON",  "4W"};

void updateAnnunciators(uint16_t state) {
  uint16_t diff = (last_state ^ state);
  uint16_t tstate = state;
  tft.setTextSize(GLYPH_FONT_SIZE);
  tft.setTextColor(LCD_YELLOW, LCD_BLACK);
  for (int i = 0; i < 11; i++) {
    if (diff & 1) {
      tft.setCursor(X_OFFSET + i * X_MULT, Y_OFFSET);
      if (tstate & 1) {
        tft.print(annunciators_text[i]);
      } else {
        tft.print("     ");
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
      tft.setCursor(GLYPH_X_OFFSET, GLYPH_Y_OFFSET - (i - 12) * GLYPH_Y_MULT);
      if (tstate & 1) {
        tft.print(annunciators_text[i]);
      } else {
        tft.print("   ");
      }
    }
    diff >>= 1;
    tstate >>= 1;
  }
  last_state = state;
}

void toggleShift() {
  tft.setTextSize(2);
  tft.setTextColor(LCD_YELLOW, LCD_BLACK);
  tft.setCursor(X_OFFSET + 11 * X_MULT, Y_OFFSET);
  if (last_state & (1 << 11)) {
    tft.print("     ");
  } else {
    tft.print(annunciators_text[11]);
  }
  last_state ^= 1 << 11;
}
