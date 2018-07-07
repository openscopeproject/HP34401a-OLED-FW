#include "../glyphs/glyphs.h"
#include "annunciators.h"
#include "config.h"
#include "display.h"

extern Display display;

namespace Annunciators {

uint16_t last_state;

struct AnnunciatorData {
  char text[6];
  uint8_t x, y;
  const Glyph *glyph;
};

const AnnunciatorData annunciator_data[] = {
    {"  *", X_OFFSET, Y_OFFSET_LOW, NULL},
    {"ADRS", X_OFFSET, Y_OFFSET_HIGH, NULL},
    {"RMT", X_OFFSET + X_MULT, Y_OFFSET_HIGH, NULL},
    {"MAN", X_OFFSET + X_MULT, Y_OFFSET_LOW, NULL},
    {"TRIG", X_OFFSET + 2 * X_MULT, Y_OFFSET_LOW, NULL},
    {"HOLD", X_OFFSET + 3 * X_MULT, Y_OFFSET_LOW, NULL},
    {"MEM", X_OFFSET + 2 * X_MULT, Y_OFFSET_HIGH, NULL},
    {"RATIO", X_OFFSET + 3 * X_MULT, Y_OFFSET_HIGH, NULL},
    {"MATH", X_OFFSET + 4 * X_MULT, Y_OFFSET_LOW, NULL},
    {"ERROR", X_OFFSET + 4 * X_MULT, Y_OFFSET_HIGH, NULL},
    {"REAR", X_OFFSET + 5 * X_MULT, Y_OFFSET_LOW, NULL},
    {"Shift", X_OFFSET + 6 * X_MULT, Y_OFFSET_LOW, NULL},
    {"Diode", X_OFFSET + 5 * X_MULT, Y_OFFSET_HIGH, &diode},
    {"Cont", X_OFFSET + 6 * X_MULT, Y_OFFSET_HIGH, &continuity},
    {"4W", X_OFFSET + 7 * X_MULT, Y_OFFSET_LOW, NULL},
};

void draw(uint8_t i) {
  if (annunciator_data[i].glyph != NULL) {
    display.pushPixels(annunciator_data[i].x, annunciator_data[i].y,
                       annunciator_data[i].glyph->width,
                       annunciator_data[i].glyph->height,
                       annunciator_data[i].glyph->data, LCD_WHITE);
  } else {
    display.setCursor(annunciator_data[i].x, annunciator_data[i].y);
    display.print(annunciator_data[i].text);
  }
}

void clear(uint8_t i) {
  if (annunciator_data[i].glyph != NULL) {
    display.fillRect(annunciator_data[i].x, annunciator_data[i].y,
                     annunciator_data[i].glyph->width,
                     annunciator_data[i].glyph->height, LCD_BLACK);
  } else {
    display.fillRect(annunciator_data[i].x, annunciator_data[i].y,
                     strlen(annunciator_data[i].text) * 6 * GLYPH_FONT_SIZE,
                     7 * GLYPH_FONT_SIZE, LCD_BLACK);
  }
}

void update(uint16_t state) {
#ifdef ANNUNCIATORS_DEBUG
  state = 0xFFFF;
#endif
  uint16_t diff = (last_state ^ state);
  uint16_t tstate = state;
  display.setTextSize(GLYPH_FONT_SIZE);
  display.setTextColor(LCD_LIGHT_GRAY, LCD_BLACK);
  for (int i = 0; i < 11; i++) {
    if (diff & 1) {
      if (tstate & 1) {
        draw(i);
      } else {
        clear(i);
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
      if (tstate & 1) {
        draw(i);
      } else {
        clear(i);
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
  display.setCursor(annunciator_data[11].x, annunciator_data[11].y);
  if (last_state & (1 << 11)) {
    clear(11);
  } else {
    draw(11);
  }
  last_state ^= 0x800;
}

void clearShift() {
  if (last_state & 0x800) {
    toggleShift();
  }
}

} // namespace
