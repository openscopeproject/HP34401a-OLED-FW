/*
This file incapsulates decoding of the CPU->FP protocol of
HP/Agilent 34401a multimeter.

Protocol is SPI-like 4 wire serial with normally high clock, second
(rising) edge capture. This decoder implements software SPI to sniff
on both input and output data lines of the bus at the same time.
*/

#ifndef __DECODER_H

#include "Arduino.h"

void startSniffing();
void sckInterrupt();
void fpsInterrupt();
void process();

#endif
