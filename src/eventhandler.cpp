#include "Arduino.h"
#include "annunciators.h"
#include "bargraph.h"
#include "config.h"
#include "display.h"
#include "eventhandler.h"

extern Display display;
char dbg_msg[100];
namespace Eventhandler {

uint8_t printed;
bool need_reset = true;
char message[15];
uint16_t blinking_chars;
uint8_t blinking_state;
volatile bool process_blink;

void blinkInterrupt() {
  process_blink = true;
}

void process() {
  if (!process_blink) return;
  process_blink = false;
  blinking_state = 1 - blinking_state;
  if (blinking_state) {
    display.setTextColor(LCD_DARK_GREY, LCD_BLACK);
  } else {
    display.setTextColor(LCD_WHITE, LCD_BLACK);
  }
  display.setTextSize(MAIN_FONT_SIZE);
  uint8_t c = 0;
  for (uint8_t i = 0; message[i] != 0; i++) {
    if (message[i] == '.' || message[i] == ',' || message[i] == ':') {
      // skip over punctuation
      continue;
    }
    if (blinking_chars & (1 << c)) {
      display.setCursor(TXTX + i * (MAIN_FONT_SIZE * 6), TXTY);
      display.print(message[i]);
    }
    c++;
  }
}

void setup() {
  Timer3.setMode(TIMER_CH3, TIMER_OUTPUT_COMPARE);
  // Value carefully hand matched by nude virgins to
  // the original blinking rate of my 34401a
  Timer3.setPeriod(272511);
  Timer3.setCompare(TIMER_CH3, 1);
  Timer3.attachInterrupt(TIMER_CH3, &blinkInterrupt);
  Timer3.pause();
}

inline void print(char c) {
  display.print(c);
  message[printed++] = c;
#ifdef DEBUG
  Serial.print(c);
#endif
}

void messageByte(uint8_t byte) {
  if (need_reset) {
    printed = 0;
    blinking_chars = 0;
    Timer3.pause();
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
    blinking_chars |= 1 << (printed - 1);
  case 0x8c:
    print(':');
    break;
  case 0x81:
    // some control character
    break;
  case 0x00:
    // end of message
    message[printed] = 0;
    for (int i = 0; i < 14 - printed; i++) {
      display.print(" ");
    }
    if (blinking_chars) {
      Timer3.resume();
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
  uint16_t ctrl = h;
  ctrl = (ctrl << 8) + l;
  switch (ctrl) {
  case 0x0049: // blink 1st char
    blinking_chars |= 0x1;
    break;
  case 0x7149: // blink 2nd char
    blinking_chars |= 0x2;
    break;
  case 0x6249: // blink 3rd char
    blinking_chars |= 0x4;
    break;
  case 0x1349: // blink 4th char
    blinking_chars |= 0x8;
    break;
  case 0x5449: // blink 5th char
    blinking_chars |= 0x10;
    break;
  case 0x2549: // blink 6th char
    blinking_chars |= 0x20;
    break;
  case 0x3649: // blink 7th char
    blinking_chars |= 0x40;
    break;
  case 0x4749: // blink 8th char
    blinking_chars |= 0x80;
    break;
  case 0x3849: // blink 9th char
    blinking_chars |= 0x100;
    break;
  case 0x4949: // blink 10th char
    blinking_chars |= 0x200;
    break;
  case 0x5A49: // blink 11th char
    blinking_chars |= 0x400;
    break;
  case 0x2B49: // blink 12th char
    blinking_chars |= 0x800;
    break;
  case 0x712B: // menu enter
    Bargraph::disable();
    break;
  case 0x002B: // exit menus
    Bargraph::enable();
    break;
  case 0x0054: // low brightness
  case 0x6254: // high brightness
  case 0x1d00: // unknown
  case 0x8000: // unknown
    break;
  default:
#ifdef DEBUG
    Serial.println("UNKNOWN CONTROL MESSAGE");
#endif
  }
  if (blinking_chars) {
    Timer3.resume();
  }
}

/*
Button codes:
            alone       with shift
DC V        5b 5b         5b e9
AC V        9d 5b         9d e9
Ohm         cf 5b         cf e9
Freq        e9 5b         e9 e9
Cont        77 5b         77 e9
Null        7d 5b         7d e9
MinMax      5b 9d         5b bb
Left        9d 9d         9d bb
Right       cf 9d         cf bb
Down        e9 9d         e9 bb
Up          bb 9d         bb bb
AutoMan     7d 9d         7d bb
Single      5b cf         5b 7d
Shift       9d cf         9d cf
*/
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
