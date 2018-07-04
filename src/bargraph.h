#ifndef __BARGRAPH_H
#define __BARGRAPH_H
#include "Arduino.h"

namespace Bargraph {

enum BarStyle {
  FULLSCALE,
  POSITIVE
};

void enable(void);
void disable(void);
void setValue(BarStyle style, int16_t value);
}

#endif
