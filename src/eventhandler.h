#ifndef __EVENTHANDLER_H
#define __EVENTHANDLER_H
#include "Arduino.h"

namespace Eventhandler {

void setup();
void messageByte(uint8_t byte);
void annunciators(uint8_t h, uint8_t l);
void control(uint8_t h, uint8_t l);
void button(uint8_t h, uint8_t l);
void updateFps(uint16_t fps);

}

#endif
