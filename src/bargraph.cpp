#include "bargraph.h"
#include "lcd.h"

#define SCALE_OFFSET_Y 50
#define SCALE_HEIGHT 5

#define BAR_OFFSET_X 8
#define BAR_CENTER_X 128
#define BAR_OFFSET_Y 58
#define BAR_HEIGHT 7
#define BAR_MAX_WIDTH 241

#define BAR_COLOR 0x7
#define SCALE_COLOR 0x7

bool enabled;
bool bar_drawn;
BarStyle current_style;
int16_t current_value;

extern LCD tft;

void clear() {
  // Clear scale
  tft.fillRect(BAR_OFFSET_X, SCALE_OFFSET_Y, BAR_MAX_WIDTH, SCALE_HEIGHT,
               LCD_BLACK);
  // Clear bar
  tft.fillRect(BAR_OFFSET_X, BAR_OFFSET_Y, BAR_MAX_WIDTH, BAR_HEIGHT,
               LCD_BLACK);
  bar_drawn = false;
  current_value = 0;
}

void drawScale(BarStyle style) {
  uint16_t x;
  switch (style) {
  case POSITIVE: {
    for (x = BAR_OFFSET_X; x <= BAR_OFFSET_X + BAR_MAX_WIDTH; x += 10)
      tft.drawFastVLine(x, SCALE_OFFSET_Y + 3, 2, SCALE_COLOR);
    for (x = BAR_OFFSET_X; x <= BAR_OFFSET_X + BAR_MAX_WIDTH; x += 20)
      tft.drawFastVLine(x, SCALE_OFFSET_Y + 2, 1, SCALE_COLOR);
    for (x = BAR_OFFSET_X; x <= BAR_OFFSET_X + BAR_MAX_WIDTH; x += 50)
      tft.drawFastVLine(x, SCALE_OFFSET_Y, 2, SCALE_COLOR);
    break;
  }
  case FULLSCALE: {
  }
    for (x = BAR_OFFSET_X; x <= BAR_OFFSET_X + BAR_MAX_WIDTH; x += 5)
      tft.drawFastVLine(x, SCALE_OFFSET_Y + 3, 2, SCALE_COLOR);
    for (x = BAR_OFFSET_X; x <= BAR_OFFSET_X + BAR_MAX_WIDTH; x += 10)
      tft.drawFastVLine(x, SCALE_OFFSET_Y + 2, 1, SCALE_COLOR);
    for (x = BAR_OFFSET_X + 20; x <= BAR_OFFSET_X + BAR_MAX_WIDTH; x += 50)
      tft.drawFastVLine(x, SCALE_OFFSET_Y, 2, SCALE_COLOR);
    break;
  }
  current_style = style;
}

void drawBarPortion(uint8_t start, uint8_t width) {
  tft.fillRect(start, BAR_OFFSET_Y, width, BAR_HEIGHT, BAR_COLOR);
}

void clearBarPortion(uint8_t start, uint8_t width) {
  tft.fillRect(start, BAR_OFFSET_Y, width, BAR_HEIGHT, LCD_BLACK);
}

void drawBar(int16_t value) {
  switch (current_style) {
  case POSITIVE: {
    // Scale down 0-1000 to 0-200
    value = value / 5;
    if (value > BAR_MAX_WIDTH - 1)
      value = BAR_MAX_WIDTH - 1;
    if (!bar_drawn) {
      // Draw full bar.
      drawBarPortion(BAR_OFFSET_X, value + 1);
    } else {
      if (current_value < value) {
        drawBarPortion(BAR_OFFSET_X + current_value + 1, value - current_value);
      } else {
        clearBarPortion(BAR_OFFSET_X + value + 1, current_value - value);
      }
    }
    break;
  }
  case FULLSCALE: {
    if (value > BAR_MAX_WIDTH / 2)
      value = BAR_MAX_WIDTH / 2;
    if (value < -BAR_MAX_WIDTH / 2)
      value = -BAR_MAX_WIDTH / 2;
    if (!bar_drawn) {
      // Draw full bar.
      if (value >= 0) {
        drawBarPortion(BAR_CENTER_X, value + 1);
      } else {
        drawBarPortion(BAR_CENTER_X + value, value + 1);
      }
    } else {
      if ((value < 0 && current_value > 0) ||
          (value > 0 && current_value < 0)) {
        // Need to clear and redraw.
        if (current_value > 0) {
          clearBarPortion(BAR_CENTER_X + 1, current_value);
          drawBarPortion(BAR_CENTER_X + value, -value);
        } else {
          clearBarPortion(BAR_CENTER_X + current_value, -current_value);
          drawBarPortion(BAR_CENTER_X + 1, value);
        }
      } else if (value > 0 || current_value > 0) {
        if (current_value < value) {
          drawBarPortion(BAR_CENTER_X + current_value + 1,
                         value - current_value);
        } else {
          clearBarPortion(BAR_CENTER_X + value + 1, current_value - value);
        }
      } else if (value < 0 || current_value < 0) {
        if (current_value > value) {
          drawBarPortion(BAR_CENTER_X + value, current_value - value);
        } else {
          clearBarPortion(BAR_CENTER_X + current_value, value - current_value);
        }
      }
    }
    break;
  }
  }
  current_value = value;
  bar_drawn = true;
}

void enableBar() {
  enabled = true;
  current_value = 0;
  drawScale(FULLSCALE);
  drawBar(0);
}

void disableBar() {
  enabled = false;
  clear();
}

void setStyle(BarStyle style) {
  clear();
  drawScale(style);
  drawBar(0);
}

void setValue(BarStyle style, int16_t value) {
  if (!enabled)
    return;
  if (current_style != style)
    setStyle(style);
  drawBar(value);
}
