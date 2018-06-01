#include "annunciators.h"
#include "lcd.h"

#define XOFFSET 20
#define XMULT 35
#define YOFFSET 200
#define GLYPHXOFFSET 450
#define GLYPHYOFFSET 180
#define GLYPHYMULT 15

uint16_t last_state;
extern LCD tft;

const char annunciators_text[][6] = {"  *",  "Adrs",  " Rmt",  " Man", "Trig",
                                     "Hold", " Mem",  "Ratio", "Math", "ERROR",
                                     "Rear", "Shift", "DIO",   "CON",  "4W"};

void updateAnnunciators(uint16_t state) {
  uint16_t diff = (last_state ^ state);
  uint16_t tstate = state;
  tft.setTextSize(1);
  tft.setTextColor(LCD_YELLOW, LCD_BLACK);
  for (int i = 0; i < 11; i++) {
    if (diff & 1) {
      tft.setCursor(XOFFSET + i * XMULT, YOFFSET);
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
      tft.setCursor(GLYPHXOFFSET, GLYPHYOFFSET - (i - 12) * GLYPHYMULT);
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
  tft.setTextSize(1);
  tft.setTextColor(LCD_YELLOW, LCD_BLACK);
  tft.setCursor(XOFFSET + 11 * XMULT, YOFFSET);
  if (last_state & (1 << 11)) {
    tft.print("     ");
  } else {
    tft.print(annunciators_text[11]);
  }
  last_state ^= 1 << 11;
}
