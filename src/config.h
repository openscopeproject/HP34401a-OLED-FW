/*
This file contains configuration constants.
*/
#ifndef __CONFIG_H

//#define DEBUG

#define MAX_SCK_DELAY 1500 // 1.5ms should be plenty for 100khz clock

#define TXTX 2
#define TXTY 10
#define MAIN_FONT_SIZE 3
#define FPSX 240
#define FPSY 0

// Annunciators and glyphs
#define X_OFFSET 0
#define X_MULT 20
#define Y_OFFSET 38
#define GLYPH_X_OFFSET 200
#define GLYPH_Y_OFFSET 20
#define GLYPH_Y_MULT 10
#define GLYPH_FONT_SIZE 1

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
