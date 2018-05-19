#include <Arduino.h>

#include "Adafruit_GFX.h" // Hardware-specific library
#include "lcd.h"
#include <SPI.h> // f.k. for Arduino-1.5.2

LCD tft;
int l = 0;

void setup() {
  GPIOB->regs->CRL=0x33333333;
  GPIOB->regs->CRH=0x33333333;
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(3000);
  uint32_t when = millis();
  //    while (!Serial) ;   //hangs a Leonardo until you connect a Serial
  if (!Serial)
    delay(1000); // allow some time for Leonardo
  Serial.println("Serial took " + String((millis() - when)) + "ms to start");
  tft.reset();
  uint16_t ID = tft.readID(); //
  Serial.print("ID = 0x");
  Serial.println(ID, HEX);
  if (ID == 0xD3D3)
    ID = 0x9481; // write-only shield
  // ID = 0x9488;   // force ID
  tft.begin(ID);
  tft.invertDisplay(true);
  tft.fillScreen(LCD_BLACK);
  tft.setRotation(1);
  tft.setTextColor(LCD_WHITE, LCD_BLACK);
  tft.setTextSize(2);
}

void loop() {
  String x = Serial.readString();
  if (x != "") {
    Serial.println(x);
    tft.println(x);
  }
  digitalWrite(LED_BUILTIN, l = !l);
}
