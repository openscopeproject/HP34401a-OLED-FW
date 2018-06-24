/*
This file contains configuration constants.
*/
#ifndef __CONFIG_H

//#define DEBUG

#define MAX_SCK_DELAY 1500 // 1.5ms should be plenty for 100khz clock

#ifdef USE_SSD1322_DISPLAY // OLED panel 256x64
#define TXTX 2
#define TXTY 10
#define MAIN_FONT_SIZE 3
#define FPSX 240
#define FPSY 0
#else // ILI9481 based 320x480 lcd display
#define TXTX 20
#define TXTY 150
#define MAIN_FONT_SIZE 5
#define FPSX 240
#define FPSY 0
#endif

// Annunciators and glyphs
#ifdef USE_SSD1322_DISPLAY
#define X_OFFSET 0
#define X_MULT 20
#define Y_OFFSET 38
#define GLYPH_X_OFFSET 200
#define GLYPH_Y_OFFSET 20
#define GLYPH_Y_MULT 10
#define GLYPH_FONT_SIZE 1
#else
#define X_OFFSET 0
#define X_MULT 40
#define Y_OFFSET 200
#define GLYPH_X_OFFSET 450
#define GLYPH_Y_OFFSET 180
#define GLYPH_Y_MULT 15
#define GLYPH_FONT_SIZE 2
#endif

// Bar graph constants
#define SCALE_OFFSET_Y 50
#define SCALE_HEIGHT 5

#define BAR_OFFSET_X 8
#define BAR_CENTER_X 128
#define BAR_OFFSET_Y 58
#define BAR_HEIGHT 7
#define BAR_MAX_WIDTH 241

#define BAR_COLOR 0x7
#define SCALE_COLOR 0x7

#endif
