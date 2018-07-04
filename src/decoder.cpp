#include "annunciators.h"
#include "bargraph.h"
#include "config.h"
#include "decoder.h"
#include "display.h"

// PB13 - SCK
// PB14 - MISO
// PB15 - MOSI

extern Display display;

namespace Decoder {

uint8_t byte_len;
volatile bool byte_ready;
volatile uint8_t input_byte, output_byte, input_acc, output_acc;
uint8_t input_buf[100];
uint8_t output_buf[100];
uint8_t buf_len, tinp;
uint32_t last_us, now_us;
uint8_t led_state = 1;
volatile bool byte_not_read;
char msg[100];
bool printing, packet;
uint8_t printed, zeros, packet_len, strstart;
int16_t barvalue = 0;
Bargraph::BarStyle style;

uint16_t last_fps;
volatile uint16_t fps, fpsc;

void sckInterrupt() {
  // mid byte power on detection
  now_us = micros();
  if (byte_len != 0 && now_us > last_us + MAX_SCK_DELAY) {
    byte_len = 0;
  }
  last_us = now_us;
  // read input and output bits
  tinp = (GPIOB->regs->IDR >> 14) & 0b11;
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

void fpsInterrupt() {
  fps = fpsc;
  fpsc = 0;
}

void startSniffing() {
  pinMode(PB13, INPUT);
  pinMode(PB14, INPUT);
  pinMode(PB15, INPUT);
  Timer4.setMode(TIMER_CH1, TIMER_OUTPUT_COMPARE);
  Timer4.setPeriod(1000000);       // in microseconds
  Timer4.setCompare(TIMER_CH1, 1); // overflow might be small
  Timer4.attachInterrupt(TIMER_CH1, &fpsInterrupt);
  attachInterrupt(PB13, &sckInterrupt, RISING);
}

void process() {
  if (byte_not_read) {
    digitalWrite(LED_BUILTIN, led_state = 1 - led_state);
    byte_not_read = false;
  }
  if (byte_ready) {
    input_buf[buf_len] = input_byte;
    output_buf[buf_len] = output_byte;
    byte_ready = false;
#ifdef DEBUG
    sprintf(msg, "0x%02x 0x%02x", input_buf[buf_len], output_buf[buf_len]);
    Serial.print(msg);
    if (input_buf[buf_len] >= 0x20 && input_buf[buf_len] <= 0x7e) {
      Serial.print(" ");
      Serial.println((char)input_buf[buf_len]);
    } else {
      Serial.println();
    }
#endif
    buf_len++;
    if (printing) {
      switch (input_buf[buf_len - 1]) {
      case 0x84:
        display.print(".");
        printed++;
        break;
      case 0x86:
        display.print(",");
        printed++;
        break;
      case 0x8d:
      // special semicolon that blinks previous char?
      case 0x8c:
        display.print(":");
        printed++;
        break;
      case 0x81:
        // some control character
        break;
      case 0x00:
        zeros++;
        if (zeros == 2) {
          printing = false;
          for (int i = 0; i < 14 - printed; i++) {
            display.print(" ");
          }
          display.setCursor(TXTX, TXTY);
          buf_len = 0;
          printed = 0;
          zeros = 0;

          barvalue = 0;
          style =
              isdigit(input_buf[2]) ? Bargraph::POSITIVE : Bargraph::FULLSCALE;
          uint16_t st, c;
          for (st = ((style == Bargraph::POSITIVE) ? strstart : strstart + 1),
              c = 0;
               c < ((style == Bargraph::POSITIVE) ? 4 : 3) && st < 8; st++) {
            if (isdigit(input_buf[st])) {
              barvalue = 10 * barvalue + input_buf[st] - '0';
              c++;
            }
          }
          if (style == Bargraph::FULLSCALE && input_buf[2] == '-')
            barvalue = -barvalue;
          Bargraph::setValue(style, barvalue);
          fpsc++;
          if (fps != last_fps) {
            last_fps = fps;
            display.setCursor(FPSX, FPSY);
            display.setTextSize(1);
            display.print(fps);
            if (fps < 10)
              display.print(' ');
          }
        }
        break;
      default:
        display.print((char)input_buf[buf_len - 1]);
        printed++;
      }
    }
    if (packet) {
      packet_len++;
      if (packet_len == 3) {
        uint16_t state = input_buf[buf_len - 2];
        state = (state << 8) + input_buf[buf_len - 3];
        Annunciators::update(state);
        packet_len = 0;
        buf_len = 0;
        packet = false;
      }
    }
    if (!packet && buf_len > 1 && input_buf[buf_len - 2] == 0x00 &&
        input_buf[buf_len - 1] == 0x7f) {
      strstart = buf_len;
      printing = true;
      display.setFont(NULL);
      display.setTextColor(LCD_WHITE, LCD_BLACK);
      display.setTextSize(MAIN_FONT_SIZE);
      display.setCursor(TXTX, TXTY);
    }
    if (!printing && buf_len > 1 && input_buf[buf_len - 2] == 0x7f &&
        input_buf[buf_len - 1] == 0x00) {
      packet = true;
    }
  }
}

} // namespace
