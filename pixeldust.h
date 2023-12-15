#ifndef PIXELDUST_H
#define PIXELDUST_H

#include <Wire.h>                 // For I2C communication
#include <Adafruit_LIS3DH.h>      // For accelerometer
#include <Adafruit_PixelDust.h>   // For sand simulation
#include <Adafruit_Protomatter.h> // For RGB matrix

void setupDust(Adafruit_Protomatter* matrix);
void loopDust(Adafruit_Protomatter* matrix);

#endif
