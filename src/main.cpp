#include <Arduino.h>

#include "Adafruit_GFX.h" // Hardware-specific library
#include "annunciators.h"
#include "bargraph.h"
#include "lcd.h"
#include "config.h"

LCD tft;
int l = 0;

// PB13 - SCK
// PB14 - MISO
// PB15 - MOSI

uint8_t byte_len;
volatile bool byte_ready;
volatile uint8_t input_byte, output_byte, input_acc, output_acc;
uint8_t input_buf[100];
uint8_t output_buf[100];
uint8_t buf_len, tinp;
uint32_t last_us, now_us;
uint8_t led_state = 1;
volatile bool byte_not_read;

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

void setup() {
  GPIOB->regs->CRL = 0x33333333;
  GPIOB->regs->CRH = 0x33333333;
  pinMode(PB13, INPUT);
  pinMode(PB14, INPUT);
  pinMode(PB15, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef DEBUG
  Serial.begin(9600);
  while (!Serial)
    delay(100);
#endif
  tft.begin();
#ifndef USE_SSD1322_DISPLAY
  tft.invertDisplay(true);
  tft.setRotation(1);
#endif
  tft.fillScreen(LCD_BLACK);
  updateAnnunciators(0xffff);
  // delay(10000);
  attachInterrupt(PB13, &sckInterrupt, RISING);
  digitalWrite(LED_BUILTIN, HIGH);
  enableBar();
}

char msg[100];
bool printing, packet;
uint8_t printed, zeros, packet_len, strstart;
int16_t barvalue = 0;
BarStyle style;

void loop() {
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
        tft.print(".");
        printed++;
        break;
      case 0x86:
        tft.print(",");
        printed++;
        break;
      case 0x8d:
      // special semicolon that blinks previous char?
      case 0x8c:
        tft.print(":");
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
            tft.print(" ");
          }
          tft.setCursor(TXTX, TXTY);
          buf_len = 0;
          printed = 0;
          zeros = 0;

          barvalue = 0;
          style = isdigit(input_buf[2]) ? POSITIVE : FULLSCALE;
          uint16_t st, c;
          for (st = ((style == POSITIVE) ? strstart : strstart + 1), c = 0;
               c < ((style == POSITIVE) ? 4 : 3) && st < 8; st++) {
            if (isdigit(input_buf[st])) {
              barvalue = 10 * barvalue + input_buf[st] - '0';
              c++;
            }
          }
          if (style == FULLSCALE && input_buf[2] == '-') barvalue = -barvalue;
          setValue(style, barvalue);
        }
        break;
      default:
        tft.print((char)input_buf[buf_len - 1]);
        printed++;
      }
    }
    if (packet) {
      packet_len++;
      if (packet_len == 3) {
        uint16_t state = input_buf[buf_len - 2];
        state = (state << 8) + input_buf[buf_len - 3];
        updateAnnunciators(state);
        packet_len = 0;
        buf_len = 0;
        packet = false;
      }
    }
    if (!packet && buf_len > 1 && input_buf[buf_len - 2] == 0x00 &&
        input_buf[buf_len - 1] == 0x7f) {
      strstart = buf_len;
      printing = true;
      tft.setFont(NULL);
      tft.setTextColor(LCD_CYAN, LCD_BLACK);
      tft.setTextSize(MAIN_FONT_SIZE);
      tft.setCursor(TXTX, TXTY);
    }
    if (!printing && buf_len > 1 && input_buf[buf_len - 2] == 0x7f &&
        input_buf[buf_len - 1] == 0x00) {
      packet = true;
    }
  }
}
