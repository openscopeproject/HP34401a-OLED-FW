#include <Arduino.h>

#include "decoder.h"
#include "display.h"
#include "bargraph.h"

Display display;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  displaySetup();
#ifdef DEBUG
  Serial.begin();
  // Wait for serial
  while (!Serial)
    delay(100);
#endif
  display.begin();
#ifndef USE_SSD1322_DISPLAY
  display.invertDisplay(true);
  display.setRotation(1);
#endif
  display.fillScreen(LCD_BLACK);
  enableBar();
  startSniffing();
}

void loop() {
  process();
}
