#include "annunciators.h"
#include "bargraph.h"
#include "config.h"
#include "decoder.h"
#include "eventhandler.h"

// PB12 - INT
// PB13 - SCK
// PB14 - MISO
// PB15 - MOSI

namespace Decoder {

uint32_t last_us, now_us;
uint8_t byte_len;
volatile bool byte_ready;
volatile bool byte_not_read;
volatile uint8_t input_byte, output_byte, input_acc, output_acc;

void sckInterrupt() {
  // mid byte power on detection
  now_us = micros();
  if (byte_len != 0 && now_us > last_us + MAX_SCK_DELAY) {
    byte_len = 0;
  }
  last_us = now_us;
  // read input and output bits
  uint8_t tinp = (GPIOB->regs->IDR >> 14) & 0b11;
  output_acc = (output_acc << 1) + (tinp >> 1);
  input_acc = (input_acc << 1) + (tinp & 1);
  byte_len++;
  if (byte_len == 8) {
    if (byte_ready) {
      byte_not_read = true;
    }
    input_byte = input_acc;
    output_byte = output_acc;
    byte_len = 0;
    byte_ready = true;
  }
}

volatile uint16_t fps, fpsc;

void fpsInterrupt() {
  fps = fpsc;
  fpsc = 0;
}

void setup() {
  pinMode(PB12, INPUT);
  pinMode(PB13, INPUT);
  pinMode(PB14, INPUT);
  pinMode(PB15, INPUT);
  Timer4.setMode(TIMER_CH4, TIMER_OUTPUT_COMPARE);
  Timer4.setPeriod(1000000);       // in microseconds
  Timer4.setCompare(TIMER_CH4, 1);
  Timer4.attachInterrupt(TIMER_CH4, &fpsInterrupt);
  Eventhandler::setup();
}

void startSniffing() {
  attachInterrupt(PB13, &sckInterrupt, RISING);
}

uint8_t input_buf[100];
uint8_t output_buf[100];
uint8_t buf_len;
uint8_t led_state = 1;
bool printing, packet;
uint8_t printed, zeros, packet_len, strstart;
int16_t barvalue = 0;
Bargraph::BarStyle style;

enum FrameState {
  INIT,         // Init state
  UNKNOWN,      // State before frame type is identified
  MESSAGE,      // State is CPU->FP message frame
  ANNUNCIATORS, // State is CPU->FP annunciators update frame
  CONTROL,      // State is CPU->FP control frame
  BUTTON        // State is FP->CPU button press frame
} frame_state = INIT;

inline bool lastBytesAreEof() {
  return input_buf[buf_len - 1] == 0x00 && output_buf[buf_len - 1] == 0xbb;
}

inline void endFrame() {
  buf_len = 0;
  frame_state = UNKNOWN;
}

void updateBarGraph() {
  style = isdigit(input_buf[2]) ? Bargraph::POSITIVE : Bargraph::FULLSCALE;
  barvalue = 0;
  uint16_t st, c;
  for (st = ((style == Bargraph::POSITIVE) ? 2 : 3), c = 0;
       c < ((style == Bargraph::POSITIVE) ? 4 : 3) && st < 8; st++) {
    if (isdigit(input_buf[st])) {
      barvalue = 10 * barvalue + input_buf[st] - '0';
      c++;
    }
  }
  if (style == Bargraph::FULLSCALE && input_buf[2] == '-')
    barvalue = -barvalue;
  Bargraph::setValue(style, barvalue);
}

void process() {
  if (byte_not_read) {
    digitalWrite(LED_BUILTIN, led_state = 1 - led_state);
    byte_not_read = false;
  }
  Eventhandler::process();
  if (!byte_ready)
    return;

  input_buf[buf_len] = input_byte;
  output_buf[buf_len] = output_byte;
  byte_ready = false;
  buf_len++;
  switch (frame_state) {
  case INIT:
    if (lastBytesAreEof()) {
      endFrame();
    }
    break;
  case UNKNOWN:
    if (buf_len == 1 && input_buf[0] == 0x00 && output_buf[0] == 0x77) {
      frame_state = BUTTON;
      break;
    }
    if (buf_len == 2) {
      if (input_buf[0] == 0x00 && (input_buf[1] & 0x7f) == 0x7f) {
        frame_state = MESSAGE;
        break;
      } else if ((input_buf[0] & 0x7f) == 0x7f && input_buf[1] == 0x00) {
        frame_state = ANNUNCIATORS;
        break;
      } else {
        frame_state = CONTROL;
        break;
      }
    }
    break;
  case MESSAGE:
    if (lastBytesAreEof()) {
      updateBarGraph();
      fpsc++;
      Eventhandler::updateFps(fps);
      endFrame();
    } else {
      Eventhandler::messageByte(input_buf[buf_len - 1]);
    }
    break;
  case ANNUNCIATORS:
    if (lastBytesAreEof()) {
      Eventhandler::annunciators(input_buf[3], input_buf[2]);
      endFrame();
    }
    break;
  case CONTROL:
    if (lastBytesAreEof()) {
      Eventhandler::control(input_buf[0], input_buf[1]);
      endFrame();
    }
    break;
  case BUTTON:
    if (input_buf[buf_len - 1] == 0x66) {
      Eventhandler::button(output_buf[1], output_buf[2]);
      endFrame();
    }
    break;
  }
}

} // namespace
