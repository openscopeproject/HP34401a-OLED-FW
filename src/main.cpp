#include <Arduino.h>

#include "decoder.h"
#include "display.h"
#include "bargraph.h"

Display display;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Decoder::setup();
  displaySetup();
#ifdef DEBUG
  Serial.begin();
  // Wait for serial
  while (!Serial)
    delay(100);
#endif
  display.begin();
  display.fillScreen(LCD_BLACK);
  Bargraph::enable();
  Decoder::startSniffing();
}

void loop() {
  Decoder::process();
}
