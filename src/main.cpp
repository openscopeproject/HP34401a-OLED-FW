#include <Arduino.h>

#include "Adafruit_GFX.h" // Hardware-specific library
#include "lcd.h"

#define MAX_SCK_DELAY 500 // 0.5ms should be plenty for 100khz clock
#define TXTX 20
#define TXTY 150

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

void sckInterrupt() {
  // mid byte power on detection
  now_us = micros();
  if (byte_len != 0 && now_us - last_us > MAX_SCK_DELAY) {
    byte_len = 0;
  }
  last_us = now_us;
  // read input and output bits
  tinp = (GPIOB->regs->IDR >> 14) & 0b11;
  output_acc = (output_acc << 1) + (tinp >> 1);
  input_acc = (input_acc << 1) + (tinp & 1);
  byte_len++;
  if(byte_len == 8) {
    input_byte = input_acc;
    output_byte = output_acc;
    byte_len = 0;
    byte_ready = true;
  }
}

void setup() {
  GPIOB->regs->CRL=0x33333333;
  GPIOB->regs->CRH=0x33333333;
  pinMode(PB13, INPUT);
  pinMode(PB14, INPUT);
  pinMode(PB15, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(3000);
  while (!Serial) delay(100);
  tft.reset();
  uint16_t ID = tft.readID();
  Serial.print("LCD ID = 0x");
  Serial.println(ID, HEX);
  tft.begin(ID);
  tft.invertDisplay(true);
  tft.fillScreen(LCD_BLACK);
  tft.setRotation(1);
  tft.setTextColor(LCD_CYAN, LCD_BLACK);
  tft.setTextSize(5);
  tft.setCursor(TXTX, TXTY);
  attachInterrupt(PB13, &sckInterrupt, RISING);
}

char msg[100];
bool printing, packet;
uint8_t printed, zeros, packet_len;

void loop() {
  if(byte_ready) {
    byte_ready = false;
    input_buf[buf_len] = input_byte;
    output_buf[buf_len] = output_byte;
    sprintf(msg, "0x%02x 0x%02x", input_buf[buf_len], output_buf[buf_len]);
    Serial.print(msg);
    if (input_buf[buf_len] >= 20 && input_buf[buf_len] <= 0x7e) {
      Serial.print(" ");
      Serial.println((char)input_buf[buf_len]);
    } else {
      Serial.println();
    }
    buf_len++;
    if (printing) {
      switch (input_buf[buf_len - 1]) {
        case 0x84:
          tft.print("."); printed++; break;
        case 0x8d:
          tft.print(":"); printed++; break;
        case 0x00:
          zeros++;
          if(zeros == 2) {
            printing = false;
            for(int i = 0; i < 14 - printed; i++) {
              tft.print(" ");
            }
            tft.setCursor(TXTX, TXTY);
            buf_len = 0;
            printed = 0;
            zeros = 0;
          }
          break;
      default:
          tft.print((char)input_buf[buf_len - 1]);
          printed++;
      }
    }
    if (packet) {
      packet_len++;
      if (packet_len == 4) {
        packet_len = 0;
        buf_len = 0;
        packet = false;
      }
    }
    if (!packet &&
        buf_len > 1 &&
        input_buf[buf_len - 2] == 0x00 &&
        input_buf[buf_len - 1] == 0x7f) {
      printing = true;
    }
    if (!printing &&
        input_buf[buf_len - 1] == 0x7f) {
      packet = true;
    }
  }
}
