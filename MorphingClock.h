#ifndef MORPHCLOCK_H
#define MORPHCLOCK_H

#include <Arduino.h>
#include <Adafruit_Protomatter.h>

void setup_MorphClock(Adafruit_Protomatter* matrix, uint16_t clockColor);
void loop_MorphClock();

#endif
