#ifndef __ANNUNCIATORS_H
#define __ANNUNCIATORS_H
#include "Arduino.h"

namespace Annunciators {

void update(uint16_t state);
void toggleShift();
void clearShift();

}

#endif
