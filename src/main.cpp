#include <Arduino.h>

#include "decoder.h"
#include "display.h"
#include "bargraph.h"
#include "config.h"

Display display;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
#ifdef DEBUG
  Serial.begin();
  // Wait for serial
  while (!Serial)
    delay(100);
#endif
  Serial.println("Decoder setup...");
  Decoder::setup();
  Serial.println("Decoder setup done");
  displaySetup();
  Serial.println("Display setup done");
  display.begin();
  display.setRotation(1);
  display.fillScreen(LCD_BLACK);
  Bargraph::enable();
  Decoder::startSniffing();
}

void loop() {
  Decoder::process();
}
