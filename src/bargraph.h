#ifndef __BARGRAPH_H
#define __BARGRAPH_H
#include "Arduino.h"

enum BarStyle {
  FULLSCALE,
  POSITIVE
};

void enableBar(void);
void disableBar(void);
void setValue(BarStyle style, int16_t value);

#endif
